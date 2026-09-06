/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

namespace io::gamepad {

constexpr Uint8 GAMEPAD_BUTTON_LEFT_TRIGGER = SDL_GAMEPAD_BUTTON_COUNT;
constexpr Uint8 GAMEPAD_BUTTON_RIGHT_TRIGGER = SDL_GAMEPAD_BUTTON_COUNT+1;


class Gamepad {
private:
	/**
	 * @brief Minimum dead zone to avoid reading stick drift
	 * @note This is the generally recommended value, but not necessarily accurate for all gamepads
	 */
	static constexpr int DEAD_ZONE = 8000;

	/**
	 * @brief State of buttons on gamepad, plus 2 entries for the triggers
	 */
	std::array<bool, GAMEPAD_BUTTON_RIGHT_TRIGGER+1> m_button_state;

	SDL_JoystickID m_id;	//!< SDL joystick/gamepad id
	SDL_Gamepad *m_gamepad;	//!< SDL gamepad handle

	/**
	 * @brief Translates a gamepad button press into a mouse button press
	 * @param button Uint8 representation of SDL_GAMEPAD_BUTTON_*
	 */
	void mark_mouse_button(Uint8 button);

	/**
	 * @brief Translates a gamepad button press into a SCPUI mouse or keyboard input
	 * @param button Uint8 representation of SDL_GAMEPAD_BUTTON_*
	 */
	void scpui_translate_button(Uint8 button);

public:
	Gamepad(SDL_JoystickID _id);
	~Gamepad();

	SDL_JoystickID getId() const { return m_id; }

	/**
	 * @brief Determines if the action button has been pressed (similar to left mouse button)
	 * @note Typically the A button on Xbox style gamepads
	 *
	 * @return @c true if the action button is pressed, @c false otherwise
	 */
	bool action();

	/**
	 * @brief Determines if the cancel button has been pressed (similar to Esc key)
	 * @note Typically the B button on Xbox style gamepads
	 *
	 * @return @c true if the cancel button is pressed, @c false otherwise
	 */
	bool cancel();

	/**
	 * @brief Determines if the action or cancel buttons have been pressed
	 *
	 * @return @c true if the action or cancel button is pressed, @c false otherwise
	 */
	bool action_or_cancel() { return (action() || cancel()); }

	/**
	 * @brief Get the keyboard equivalent of a gamepad button
	 *
	 * @return KEY_* value corresponding to gamepad button
	 */
	int get_key();

	/**
	 * @brief Gets the down time of the given hat and position
	 * @param[in] button Uint8 representation of SDL_GAMEPAD_BUTTON_*
	 * @param[in] down @c true if button is pressed, @c false if button is released
	 */
	void mark_button(Uint8 button, bool down);

	/**
	 * @brief Update mouse/cursor position based on gamepad stick movement
	 */
	bool update_mouse_pos();

	/**
	 * @brief Translate gamepad state into mouse states for camera/object movement
	 *
	 * @details These controls were chosen due to ImGui not using them by default. If those defaults
	 * change an alternate mapping may be required.
	 *
	 * @param[out] dx x-axis delta of right stick
	 * @param[out] dy y-axis delta of right stick
	 * @param[out] dz z-axis delta (mouse wheel, only if right trigger pressed)
	 * @param[out] lmb_down @c true if left mouse button down ( dx/dy has value and no triggers pressed)
	 * @param[out] rmb_down @c true if riight mouse button down (dx/dy has value and left trigger pressed)
	 * @param[out] key_flags will have @c KEY_SHIFTED set if both triggers are pressed
	 *
	 * @returns @c true if any of the values were set
	 */
	bool get_camera_vals(int *dx, int *dy, int *dz, bool *lmb_down, bool *rmb_down, uint *key_flags);
};

void init();
void shutdown();
void do_frame();

/**
 * @brief Returns @c true if gamepad navigation is enabled and active
 */
bool navActive();

/**
 * @brief Returns @c true if the defined Action button has been pressed
 */
bool action();

/**
 * @brief Returns @c true if the defined Cancel button has been pressed
 */
bool cancel();

/**
 * @brief Returns @c true if the defined Action or Cancel buttons have been pressed
 */
bool action_or_cancel();

/**
 * @brief Get the keyboard equivalent of any pressed gamepad buttons
 *
 * @return KEY_* value corresponding to a gamepad button
 */
int get_key();

/**
 * @brief Translate gamepad state into mouse states for camera/object movement
 *
 * @details These controls were chosen due to ImGui not using them by default. If those defaults
 * change an alternate mapping may be required.
 *
 * @param[out] dx x-axis delta of right stick
 * @param[out] dy y-axis delta of right stick
 * @param[out] dz z-axis delta (mouse wheel, only if right trigger pressed)
 * @param[out] lmb_down @c true if left mouse button down ( dx/dy has value and no triggers pressed)
 * @param[out] rmb_down @c true if riight mouse button down (dx/dy has value and left trigger pressed)
 * @param[out] key_flags will have @c KEY_SHIFTED set if both triggers are pressed
 *
 * @returns @c true if any of the values were set to something other than defaults
 *
 * @warning This function sets all passed arguments to default values if gamepad navigation is active!
 */
bool get_camera_vals(int *dx, int *dy, int *dz, bool *lmb_down, bool *rmb_down, uint *key_flags);
}

#endif

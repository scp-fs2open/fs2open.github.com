#ifndef _MOUSE_H
#define _MOUSE_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"


extern int Mouse_sensitivity;
extern int Use_mouse_to_fly;

#define MOUSE_LEFT_BUTTON   (1<<0)
#define MOUSE_RIGHT_BUTTON  (1<<1)
#define MOUSE_MIDDLE_BUTTON (1<<2)
#define MOUSE_X1_BUTTON     (1<<3)
#define MOUSE_X2_BUTTON     (1<<4)
#define MOUSE_WHEEL_UP      (1<<5)  //!< [z64555] Wheel is treated like a pair of buttons, but they don't have a down nor up count
#define MOUSE_WHEEL_DOWN    (1<<6)
#define MOUSE_WHEEL_LEFT    (1<<7)
#define MOUSE_WHEEL_RIGHT   (1<<8)

#define MOUSE_NUM_BUTTONS		9

// [:V:] keep the following #defines up to date with the #defines above
// [z64555] These mouse_button defines are a quick check to verify the input button is a mouse button
// [z64555] Likewise, the mouse_wheel defines quickly check against the mousewheel directions
#define LOWEST_MOUSE_BUTTON     MOUSE_LEFT_BUTTON
#define HIGHEST_MOUSE_BUTTON    MOUSE_X2_BUTTON

#define LOWEST_MOUSE_WHEEL      MOUSE_WHEEL_UP
#define HIGHEST_MOUSE_WHEEL     MOUSE_WHEEL_RIGHT


/**
 * @brief Inits the mouse
 */
void mouse_init();

/**
 * @brief Marks a mouse button as up or down
 *
 * @param[in] flags Which button(s) are pressed/released
 * @param[in] set   Direction of the button(s). 1 = pressed, 0 = released
 *
 * @note This function is extern'ed by osapi.cpp and freespace.cpp
 */
void mouse_mark_button(uint flags, int set);

/**
 * @brief Gets the current mouse position clipped to screen coordinates
 *
 * @param[out] xpos If not NULL, The x (left/right) position
 * @param[out] ypos If not NULL, The y (up/down) position
 *
 * @returns Button states as a bitfield
 */
int mouse_get_pos(int *xpos, int *ypos);

/**
 * @brief Gets the current (unscaled) mouse position clipped to screen coordinates
 *
 * @param[out] xpos If not NULL, The x (left/right) position
 * @param[out] ypos If not NULL, The y (up/down) position
 *
 * @returns Button states as a bitfield
 */
int mouse_get_pos_unscaled(int *xpos, int *ypos);

/**
 * @brief Gets the current mouse position (OS coordinates). Can be negative.
 *
 * @param[out] mx If not NULL, The x (left/right) position
 * @param[out] my If not NULL, The y (up/down) position
 */
void mouse_get_real_pos(int *mx, int *my);

/**
 * @brief Move the mouse cursor to the specified coordinates
 *
 * @param[in] xpos The new x position
 * @param[in] ypos The new y position
 */
void mouse_set_pos(int xpos, int ypos);

/**
 * @brief Returns the number of times button n went from up to down since last call
 */
int mouse_down_count(int n, int reset_count = 1);

/**
 * @brief Returns the number of times button n went from down to up since last call
 */
int mouse_up_count(int n);

/**
 * @brief Flushes the mouse's states clean. Called whenever it gains/loses focus
 */
void mouse_flush();

/**
 * @brief Checks if the given mouse button(s) are down
 *
 * @returns 1 if true, or
 * @returns 0 otherwise
 *
 * @details Also covers mousewheels, and applies decay
 */
int mouse_down(int btn);

/**
 * @brief Returns the time of how long the button has been since last call
 *
 * @note Currently just returns 1.0f if down, 0.0f if not.
 *
 * @details Also covers mousewheels, and applies decay
 */
float mouse_down_time(int btn);

/**
 * @brief Handler called when we loose focus
 *
 * @note Currently unused
 */
void mouse_lost_focus();

/**
 * @brief Handler called when we gain focus
 *
 * @note Currently unused
 */
void mouse_got_focus();

/**
 * @brief Saves the current deltas into Last_mouse deltas, and clears the current deltas
 */
void mouse_reset_deltas();

/**
 * @brief Gets the current mouse deltas
 *
 * @param[out] dx If not NULL, The delta x
 * @param[out] dy If not NULL, The delta y
 * @param[out] dz If not NULL, The delta z (usually 0)
 */
void mouse_get_delta(int *dx = NULL, int *dy = NULL, int *dz = NULL);

/**
 * @brief Handler called when a mouse movement even occurs
 *
 * param[in] x  The current X position
 * param[in] y  The current Y position
 * param[in] dx The current delta X
 * param[in] dy The current delta Y
 */
void mouse_event(int x, int y, int dx, int dy);

/**
 * @brief Called when there is motion on the mouse wheel(s). Supports 2 axes
 *
 * @param[in] x Current mousewheel movement along the X axis (left/right)
 * @param[in] y Current mousewheel movement along the Y axis (up/down)
 */
void mousewheel_motion(int x, int y);

#endif

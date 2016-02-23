/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_KEYCONTROL_H__
#define __FREESPACE_KEYCONTROL_H__

#include "controlconfig/controlsconfig.h"

/**
 * @brief Number of int32_t's in button_info's status[]
 *
 * @details Since CCFG_MAX is garunteed to be a value of at least 1, add 31 to ensure a minimum of 1x int32_t
 */
#define NUM_BUTTON_FIELDS	((CCFG_MAX + 31) / 32)

extern int Dead_key_set[];      //!< Set of controls available while dead
extern int Dead_key_set_size;   //!< Dead_key_set.size()
extern bool Perspective_locked; //!< If true, prevent the player from being able to switch perspectives (such as between 3rd person and cockpit)

extern int Ignored_keys[];      //!< Set of controls that are ignored. Counter based. 0 Means don't ignore this control

extern bool quit_mission_popup_shown;   //!< True if the "Quit Mission" popup is visible/in focus

/**
 * @brief Structure containing the activation status of all controls
 *
 * @details Basically a bitset. Active controls have set bits and inactive controls have cleared bits.
 */
typedef struct button_info
{
	int status[NUM_BUTTON_FIELDS];
} button_info;

/**
 * @brief Set the bit in button_info for the given IoActionId
 *
 * @param[in] n The IoActionId
 */
void button_info_set(button_info *bi, int n);

/**
 * @brief Clear the bit in button_info for the given IoActionId
 *
 * @param[in] n The IoActionId
 */
void button_info_unset(button_info *bi, int n);

/**
 * @brief Returns the state of the associated IoActionId bit in button_info
 *
 * @param[in] n The IoActionId
 */
bool button_info_query(button_info *bi, int n);

/**
 * @brief Calls the event handlers for each active button in button_info
 */
void button_info_do(button_info *bi);

/**
 * @brief Clears the entire button_info struct
 */
void button_info_clear(button_info *bi);

/**
 * @brief Marks the key/button in the player's button_info if it is in the given control list
 *
 * @param[in] key   Scancode (plus modifiers)
 * @param[in] count Total size of the list
 * @param[in] list  List of ::Control_config struct action indices to check for
 *
 * @details Also checks joystick controls in the set.
 */
void process_set_of_keys(int key, int count, int *list);

/**
 * @brief Handle pause keypress
 */
void game_process_pause_key();

/**
 * @brief Strip out all noncritical keys from the ::button_info struct
 */
void button_strip_noncritical_keys(button_info *bi);


#endif

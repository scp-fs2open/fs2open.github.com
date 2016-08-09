#ifndef _WING_H
#define _WING_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */



#include "management.h"


/**
 * @brief Forms a wing from marked objects
 *
 * @returns  0 If successful, or
 * @returns -1 If an error occured
 */
int create_wing();

/**
 * @brief Delete a whole wing, leaving ships intact but wingless.
 *
 * @param[in] wing_num Index of the wing
 */
void remove_wing(int wing_num);

/**
 * @brief Takes a ship out of a wing, deleting the wing if that was the only ship in it.
 *
 * @param[in] ship Index of the ship to remove (Ships[i])
 * @param[in] min  Minimum number of ships in a wing.
 *   Pass a 0 to allow a wing to exist without any ships in it, or pass a value >1 to have the wing deleted when it has
 *   this many members in it
 */
void remove_ship_from_wing(int ship, int min = 1);

/**
 * @brief Mark all ships within this wing
 *
 * @param[in] wing Index of the wing to mark
 */
void mark_wing(int wing);

/**
 * @brief Delete the whole wing, and its ships if necassary.
 *
 * @param[in] wing  Optional. Index of the wing to delete. If not specified then deletes the "current" wing
 * @param[in] bypass Bool. Optional. If nonzero then don't delete the ships
 *
 * @returns 0 If successful, or
 * @returns nonzero if could not delete wing
 */
int delete_wing(int wing = cur_wing, int bypass = 0);

#endif // _WING_H

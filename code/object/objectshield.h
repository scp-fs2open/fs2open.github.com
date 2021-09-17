/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _OBJECTSHIELD_H
#define _OBJECTSHIELD_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "object/object.h"

#define	FRONT_QUAD	1
#define	REAR_QUAD	2
#define	LEFT_QUAD	3
#define	RIGHT_QUAD	0

/**
 * @brief Balances/Equalizes the shield
 *
 * @param[in] balance_rate The rate to balance the shield (0 < x <= 1)
 * @param[in] penalty      Percent of the shield energy to lose during the balance (0 < x <= 1)
 *
 * @author z64555
 *
 * @details Each quadrant is observed to exponentially approach the average shield HP, weaker quadrants quickly fill
 *   up while stronger quadrants fille up slower. We don't have to worry about the shield gaining energy from
 *   somewhere, since the sum of each (shield_hp_avg - shield_quadrant[i]) is 0.
 *   Applying a balance rate is legal since it's equivalent to applying the rate to the entire sum, which is still 0.
 *
 * @TODO Verify operation with model point shields
 */
void shield_balance(object *objp, float rate, float penalty);

/**
 * @brief Transfers energy to the given quadrant from the other quadrants
 *
 * @param[in] quadrant Index of the quadrant to Xfer to
 * @param[in] rate     Percent rate to increase the target quadrant
 *
 * @author z64555, zookeeper
 *
 * @details The transfer sips a percentage of each quadrant, taking more energy from stronger quadrants and less from
 *   weaker ones.
 */
void shield_transfer(object *objp, int quadrant, float rate);

/**
 * @brief Gets the shield strength (in HP) of the given object
 */
float shield_get_strength(object *objp);

/**
 * @brief Sets the shield strength (in HP) of the given object.
 * @note All quadrants are set to an equal strength
 */
void shield_set_strength(object *objp, float strength);

/**
 * @brief Adds the given delta to the given object's shield.
 *
 * @param[in] delta HP to add (or subtract if negative) to the object's shield
 */
void shield_add_strength(object *objp, float delta);

/**
 * @brief Gets the strength (in HP) of a shield quadrant/sector
 *
 * @param[in] quadrant_num Index of the quadrant/sector to check.
 *
 * @author Goober5000
 */
float shield_get_quad(object *objp, int quadrant_num);

/**
 * @brief Sets the strength (in HP) of a shield quadrant/sector
 *
 * @param[in] quadrant_num Index of the quadrant/sector to set.
 * @param[in] strength HP to set
 *
 * @author Goober5000
 */
void shield_set_quad(object *objp, int quadrant_num, float strength);

/**
 * @brief Sets the strength (in HP) of a shield quadrant/sector
 *
 * @param[in] quadrant_num Index of the quadrant/sector to set.
 * @param[in] strength HP to add (or subtract if negative) to the quadrant/sector
 *
 * @author Goober5000
 */
void shield_add_quad(object *objp, int quadrant_num, float strength);

/**
 * @brief Gets the max shield HP of the given object
 *
 * @note $Max Shield Recharge is not intended to affect max strength of individual shield segments
 *
 * @author Goober5000
 */
float shield_get_max_strength(object *objp, bool no_msr = false);

/**
 * @brief Sets the max shield HP of the given object. Use this to init or override a ship's default shield HP
 */
void shield_set_max_strength(object *objp, float newmax);

/**
 * @brief Gets the max shield HP that a quadrant/sector may have
 *
 * @author Goober5000
 */
float shield_get_max_quad(object *objp);

/**
 * @brief Strengthens the weakest quadrant first, then spreads it out
 */
void shield_apply_healing(object* objp, float healing);

/**
 * @brief Applies damage to the given shield quandrant/sector of the given object.
 *
 * @returns Any remaining damage after being absorbed by the shield.
 *
 * @details Basically:
 *   if (Damage > Quadrant HP) {
 *     Quadrant HP = 0;
 *     return Damage - Quadrant HP;
 *   } else {
 *     Quadrant HP -= Damage;
 *     return 0.0f;
 *   }
 */
float shield_apply_damage(object *objp, int quadrant, float damage);

#endif //_OBJECTSHIELD_H

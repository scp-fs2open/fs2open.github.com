#pragma once

#include <optional>

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "fireball/fireballs.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

template<int type>
inline auto obj_get_instance_maybe(const object* objp) {
	if constexpr(type == OBJ_SHIP)
		return objp->type == type ? std::optional(&Ships[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_WEAPON)
		return objp->type == type ? std::optional(&Weapons[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_FIREBALL)
		return objp->type == type ? std::optional(&Fireballs[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_DEBRIS)
		return objp->type == type ? std::optional(&Debris[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_WING)
		return objp->type == type ? std::optional(&Wings[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_ASTEROID)
		return objp->type == type ? std::optional(&Asteroids[objp->instance]) : std::nullopt;
	else if constexpr(type == OBJ_BEAM)
		return objp->type == type ? std::optional(&Beams[objp->instance]) : std::nullopt;
	else
		return std::nullopt;
}

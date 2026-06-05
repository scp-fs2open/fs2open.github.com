//
//

#include "coordinatepoint.h"
#include "object.h"

#include "coordinate_points/coordinate_point.h"
#include "globalincs/globals.h"
#include "hud/hudescort.h"

namespace scripting::api {

//**********HANDLE: CoordinatePoint
ADE_OBJ_DERIV(l_CoordinatePoint, object_h, "coordinatepoint", "Coordinate point handle", l_Object);

ADE_VIRTVAR(Name,
	l_CoordinatePoint,
	"string",
	"Coordinate point name. Used to reference the point from SEXPs.",
	"string",
	"Coordinate point name, or empty string if handle is invalid")
{
	object_h* objh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_CoordinatePoint.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ade_set_error(L, "s", "");

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr)
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && s != nullptr) {
		cp->name = s;
	}

	return ade_set_args(L, "s", cp->name.c_str());
}

ADE_VIRTVAR(Group,
	l_CoordinatePoint,
	"string",
	"Designer-defined group string (may be empty).",
	"string",
	"Coordinate point group, or empty string if handle is invalid")
{
	object_h* objh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_CoordinatePoint.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ade_set_error(L, "s", "");

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr)
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && s != nullptr) {
		cp->group = s;
	}

	return ade_set_args(L, "s", cp->group.c_str());
}

ADE_VIRTVAR(EscortPriority,
	l_CoordinatePoint,
	"number",
	"Escort-list priority. 0 (default) means the point is not on the escort list. Any value >0 means the point is on the list, with this number used as the sort key (higher = higher on the list).",
	"number",
	"Escort priority, or 0 if handle is invalid")
{
	object_h* objh;
	int val = 0;
	if (!ade_get_args(L, "o|i", l_CoordinatePoint.GetPtr(&objh), &val))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ade_set_error(L, "i", 0);

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr)
		return ade_set_error(L, "i", 0);

	if (ADE_SETTING_VAR) {
		cp->escort_priority = (val < 0) ? 0 : val;
	}

	return ade_set_args(L, "i", cp->escort_priority);
}

ADE_VIRTVAR(VisibleInMission,
	l_CoordinatePoint,
	"boolean",
	"Whether this coordinate point's shape renders in-game (not just in the editor). Allows the player to target it with the target-in-front reticle. Defaults to false.",
	"boolean",
	"true if visible in mission, false otherwise (also false if handle is invalid)")
{
	object_h* objh;
	bool val = false;
	if (!ade_get_args(L, "o|b", l_CoordinatePoint.GetPtr(&objh), &val))
		return ADE_RETURN_FALSE;

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ADE_RETURN_FALSE;

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr)
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		cp->flags.set(CoordinatePoint::Flags::Visible_in_mission, val);
	}

	return ade_set_args(L, "b", cp->flags[CoordinatePoint::Flags::Visible_in_mission]);
}

ADE_VIRTVAR(MultiTeam,
	l_CoordinatePoint,
	"number",
	"Multiplayer team filter. -1 (default) = visible to all teams. 0 or higher = visible only to that TVT team in multiplayer. Singleplayer always renders the point regardless of this value.",
	"number",
	"Multi team value, or -1 if handle is invalid")
{
	object_h* objh;
	int val = -1;
	if (!ade_get_args(L, "o|i", l_CoordinatePoint.GetPtr(&objh), &val))
		return ade_set_error(L, "i", -1);

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ade_set_error(L, "i", -1);

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr)
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR) {
		if (val < -1 || val >= MAX_TVT_TEAMS) {
			val = -1;
		}
		cp->multi_team = val;
	}

	return ade_set_args(L, "i", cp->multi_team);
}

ADE_FUNC(addToEscortList, l_CoordinatePoint, nullptr,
	"Adds this coordinate point to the player's escort list. Requires EscortPriority > 0; otherwise a no-op.",
	"boolean",
	"true if the point was added (or already on the list), false if EscortPriority is 0 / handle invalid")
{
	object_h* objh;
	if (!ade_get_args(L, "o", l_CoordinatePoint.GetPtr(&objh)))
		return ADE_RETURN_FALSE;

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ADE_RETURN_FALSE;

	auto* cp = find_coordinate_point_by_objnum(objh->objnum);
	if (cp == nullptr || cp->escort_priority <= 0)
		return ADE_RETURN_FALSE;

	hud_add_ship_to_escort(objh->objnum, 1);
	return ADE_RETURN_TRUE;
}

ADE_FUNC(removeFromEscortList, l_CoordinatePoint, nullptr,
	"Removes this coordinate point from the player's escort list (no-op if it isn't on it).",
	"boolean",
	"true if a removal was issued, false if handle invalid")
{
	object_h* objh;
	if (!ade_get_args(L, "o", l_CoordinatePoint.GetPtr(&objh)))
		return ADE_RETURN_FALSE;

	if (!objh->isValid() || objh->objp()->type != OBJ_COORDINATE_POINT)
		return ADE_RETURN_FALSE;

	hud_remove_ship_from_escort(objh->objnum);
	return ADE_RETURN_TRUE;
}

} // namespace scripting::api

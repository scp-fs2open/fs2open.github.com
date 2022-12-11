//
//

#include "oswpt.h"
#include "parse_object.h"
#include "ship.h"
#include "team.h"
#include "waypoint.h"
#include "wing.h"
#include "ship/ship.h"


#define OSWPT_TYPE_NONE				oswpt_type::NONE
#define OSWPT_TYPE_SHIP				oswpt_type::SHIP
#define OSWPT_TYPE_WING				oswpt_type::WING
#define OSWPT_TYPE_WAYPOINT			oswpt_type::WAYPOINT
#define OSWPT_TYPE_SHIP_ON_TEAM		oswpt_type::SHIP_ON_TEAM
#define OSWPT_TYPE_WHOLE_TEAM		oswpt_type::WHOLE_TEAM
#define OSWPT_TYPE_PARSE_OBJECT		oswpt_type::PARSE_OBJECT
#define OSWPT_TYPE_EXITED			oswpt_type::EXITED
#define OSWPT_TYPE_WING_NOT_PRESENT	oswpt_type::WING_NOT_PRESENT


namespace scripting {
namespace api {

//**********HANDLE: Wing
ADE_OBJ(l_OSWPT, object_ship_wing_point_team, "oswpt", "Handle for LuaSEXP arguments that can hold different types (Object/Ship/Wing/Waypoint/Team)");

ADE_FUNC(getType, l_OSWPT, nullptr, "The data-type this OSWPT yields on the get method.", "string", "The name of the data type. Either 'ship', 'parseobject' (a yet-to-spawn ship), 'wing' (can include yet-to-arrive wings with 0 current ships), 'team' (both explicit and ship-on-team), 'waypoint',  or 'none' (either explicitly specified, a ship that doesn't exist anymore, or invalid OSWPT object).")
{
	object_ship_wing_point_team oswpt;
	if (!ade_get_args(L, "o", l_OSWPT.Get(&oswpt)))
		return ade_set_error(L, "s", "none");

	switch (oswpt.type) {
	case OSWPT_TYPE_SHIP:
		return ade_set_args(L, "s", "ship");
	case OSWPT_TYPE_PARSE_OBJECT:
		return ade_set_args(L, "s", "parseobject");
	case OSWPT_TYPE_WING:
	case OSWPT_TYPE_WING_NOT_PRESENT:
		return ade_set_args(L, "s", "wing");
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM:
		return ade_set_args(L, "s", "team");
	case OSWPT_TYPE_WAYPOINT:
		return ade_set_args(L, "s", "waypoint");
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		return ade_set_args(L, "s", "none");
	}
}

ADE_FUNC(get, l_OSWPT, nullptr, "Returns the data held by this OSWPT.", "ship | parse_object | wing | team | waypoint | nil", "Returns the data held by this OSWPT, nil if type is 'none'.")
{
	object_ship_wing_point_team oswpt;
	if (!ade_get_args(L, "o", l_OSWPT.Get(&oswpt)))
		return ADE_RETURN_NIL;

	switch (oswpt.type) {
	case OSWPT_TYPE_SHIP:
		return ade_set_args(L, "o", l_Ship.Set(object_h(oswpt.objp)));
	case OSWPT_TYPE_PARSE_OBJECT:
		return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(oswpt.ship_entry->p_objp)));
	case OSWPT_TYPE_WING:
	case OSWPT_TYPE_WING_NOT_PRESENT:
		return ade_set_args(L, "o", l_Wing.Set(WING_INDEX(oswpt.wingp)));
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM:
		return ade_set_args(L, "o", l_Team.Set(oswpt.team));
	case OSWPT_TYPE_WAYPOINT:
		return ade_set_args(L, "o", l_Waypoint.Set(object_h(oswpt.objp)));
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(forAllShips, l_OSWPT, "function(ship ship) => void body", "Applies this function to each (present) ship this OSWPT applies to.", nullptr, nullptr)
{
	object_ship_wing_point_team oswpt;
	luacpp::LuaFunction body;
	if (!ade_get_args(L, "ou", l_OSWPT.Get(&oswpt), &body))
		return ADE_RETURN_NIL;

	switch (oswpt.type) {
	case OSWPT_TYPE_SHIP:
	{
		luacpp::LuaValueList args;
		args.push_back(luacpp::LuaValue::createValue(L, l_Ship.Set(object_h(oswpt.objp))));

		body.call(L, args);
		break;
	}
	case OSWPT_TYPE_WING:
	{
		auto wp = oswpt.wingp;
		for (int i = 0; i < wp->current_count; ++i)
		{
			luacpp::LuaValueList args;
			args.push_back(luacpp::LuaValue::createValue(L, l_Ship.Set(object_h(&Objects[Ships[wp->ship_index[i]].objnum]))));

			body.call(L, args);
		}
		break;
	}
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM:
		for (auto so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
		{
			if (Ships[Objects[so->objnum].instance].team == oswpt.team)
			{
				luacpp::LuaValueList args;
				args.push_back(luacpp::LuaValue::createValue(L, l_Ship.Set(object_h(&Objects[so->objnum]))));

				body.call(L, args);
			}
		}
		break;
	case OSWPT_TYPE_PARSE_OBJECT:
	case OSWPT_TYPE_WING_NOT_PRESENT:
	case OSWPT_TYPE_WAYPOINT:
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		break;
	}
	
	return ADE_RETURN_NIL;
}


ADE_FUNC(forAllParseObjects, l_OSWPT, "function(parse_object po) => void body", "Applies this function to each not-yet-present ship (includes not-yet-present wings and not-yet-present ships of a specified team!) this OSWPT applies to.", nullptr, nullptr)
{
	object_ship_wing_point_team oswpt;
	luacpp::LuaFunction body;
	if (!ade_get_args(L, "ou", l_OSWPT.Get(&oswpt), &body))
		return ADE_RETURN_NIL;

	switch (oswpt.type) {
	case OSWPT_TYPE_PARSE_OBJECT:
	{
		luacpp::LuaValueList args;
		args.push_back(luacpp::LuaValue::createValue(L, l_ParseObject.Set(parse_object_h(oswpt.ship_entry->p_objp))));

		body.call(L, args);
		break;
	}
	case OSWPT_TYPE_WING_NOT_PRESENT:
	{
		for (p_object* p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
		{
			if (p_objp->wingnum == WING_INDEX(oswpt.wingp))
			{
				luacpp::LuaValueList args;
				args.push_back(luacpp::LuaValue::createValue(L, l_ParseObject.Set(parse_object_h(p_objp))));

				body.call(L, args);
			}
		}
		break;
	}
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM:
	{
		for (p_object* p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
		{
			if (p_objp->team == oswpt.team)
			{
				luacpp::LuaValueList args;
				args.push_back(luacpp::LuaValue::createValue(L, l_ParseObject.Set(parse_object_h(p_objp))));

				body.call(L, args);
			}
		}
		break;
	}
	case OSWPT_TYPE_SHIP:
	case OSWPT_TYPE_WING:
	case OSWPT_TYPE_WAYPOINT:
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		break;
	}

	return ADE_RETURN_NIL;
}


}
}

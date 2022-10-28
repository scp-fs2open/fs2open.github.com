//
//

#include "oswpt.h"
#include "parse_object.h"
#include "ship.h"
#include "team.h"
#include "waypoint.h"
#include "wing.h"
#include "ship/ship.h"
#include "mission/missionparse.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

void object_ship_wing_point_team::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	object_ship_wing_point_team oswpt;
	value.getValue(scripting::api::l_OSWPT.Get(&oswpt));
	ADD_INT(oswpt.type);
	switch (oswpt.type) {
	case OSWPT_TYPE_SHIP:
	case OSWPT_TYPE_WAYPOINT:
		ADD_USHORT(oswpt.objp->net_signature);
		break;
	case OSWPT_TYPE_PARSE_OBJECT:
		ADD_USHORT(oswpt.ship_entry->p_objp->net_signature);
		break;
	case OSWPT_TYPE_WING:
	case OSWPT_TYPE_WING_NOT_PRESENT: {
		int wingidx = WING_INDEX(oswpt.wingp);
		ADD_INT(wingidx);
		break;
	}
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM:
		ADD_INT(oswpt.team);
		break;
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		break;
	}
}

void object_ship_wing_point_team::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	object_ship_wing_point_team oswpt;
	GET_INT(oswpt.type);
	switch (oswpt.type) {
	case OSWPT_TYPE_SHIP:
	case OSWPT_TYPE_WAYPOINT: {
		ushort net_signature;
		GET_USHORT(net_signature);
		object_ship_wing_point_team oswpt;
		//This doesn't constitute a valid waypoint oswpt, but it will work for everything that lua has access to, so it's fine
		oswpt.objp = multi_get_network_object(net_signature);
		new(data_ptr) object_ship_wing_point_team(std::move(oswpt));
		break;
	}
	case OSWPT_TYPE_PARSE_OBJECT: {
		ushort net_signature;
		GET_USHORT(net_signature);
		GET_USHORT(net_signature);
		new(data_ptr) object_ship_wing_point_team(mission_parse_get_arrival_ship(net_signature));
		break;
	}
	case OSWPT_TYPE_WING:
	case OSWPT_TYPE_WING_NOT_PRESENT: {
		int wingidx;
		GET_INT(wingidx);
		new(data_ptr) object_ship_wing_point_team(&Wings[wingidx]);
		break;
	}
	case OSWPT_TYPE_SHIP_ON_TEAM:
	case OSWPT_TYPE_WHOLE_TEAM: {
		int oswpt_team;
		GET_INT(oswpt_team);
		object_ship_wing_point_team oswpt;
		oswpt.team = oswpt_team;
		new(data_ptr) object_ship_wing_point_team(std::move(oswpt));
		break;
	}
	case OSWPT_TYPE_NONE:
	case OSWPT_TYPE_EXITED:
	default:
		new(data_ptr) object_ship_wing_point_team;
		break;
	}
}


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

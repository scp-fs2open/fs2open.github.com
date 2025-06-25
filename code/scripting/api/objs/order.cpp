//
//

#include "order.h"
#include "enums.h"
#include "object.h"
#include "subsystem.h"
#include "waypoint.h"

#include "ai/aigoals.h"
#include "weapon/weapon.h"
#include "playerman/player.h"

namespace scripting {
namespace api {


order_h::order_h()
	: objh(), odx(-1), sig(-1), aigp(nullptr)
{}
order_h::order_h(object* objp, int n_odx)
{
	objh = object_h(objp);
	if(objh.isValid() && objh.objp()->type == OBJ_SHIP && n_odx > -1 && n_odx < MAX_AI_GOALS)
	{
		odx = n_odx;
		sig = Ai_info[Ships[objh.objp()->instance].ai_index].goals[odx].signature;
		aigp = &Ai_info[Ships[objh.objp()->instance].ai_index].goals[odx];
	}
	else
	{
		odx = -1;
		sig = -1;
		aigp = NULL;
	}
}
bool order_h::isValid() const
{
	if (!objh.isValid() || aigp == NULL)
		return false;

	return objh.objp()->type == OBJ_SHIP && odx > -1 && odx < MAX_AI_GOALS && sig == Ai_info[Ships[objh.objp()->instance].ai_index].goals[odx].signature;
}

//**********HANDLE: order
ADE_OBJ(l_Order, order_h, "order", "order handle");

ADE_VIRTVAR(Priority, l_Order, "number", "Priority of the given order", "number", "Order priority or 0 if invalid")
{
	order_h *ohp = NULL;
	int priority = 1;

	if(!ade_get_args(L, "o|i", l_Order.GetPtr(&ohp), &priority))
		return ade_set_error(L, "i", 0);

	if(!ohp->isValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR) {
		ohp->aigp->priority = priority;
	}

	return ade_set_args(L, "i", ohp->aigp->priority);
}

ADE_FUNC(remove, l_Order, NULL, "Removes the given order from the ship's priority queue.", "boolean", "True if order was successfully removed, otherwise false or nil.")
{
	order_h *ohp = NULL;
	if(!ade_get_args(L, "o", l_Order.GetPtr(&ohp)))
		return ADE_RETURN_NIL;

	if(!ohp->isValid())
		return ADE_RETURN_FALSE;

	ai_info *aip = &Ai_info[Ships[ohp->objh.objp()->instance].ai_index];

	ai_remove_ship_goal(aip, ohp->odx);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getType, l_Order, NULL, "Gets the type of the order.", "enumeration", "The type of the order as one of the ORDER_* enumerations.")
{
	order_h *ohp = NULL;
	lua_enum eh_idx = ENUM_INVALID;
	if(!ade_get_args(L, "o", l_Order.GetPtr(&ohp)))
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	if(!ohp->isValid())
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	switch(ohp->aigp->ai_mode){
		case AI_GOAL_NONE:
		case AI_GOAL_SCHROEDINGER:
		case AI_GOAL_NUM_VALUES:
			break;
		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_CHASE_WEAPON:
		case AI_GOAL_CHASE:
			eh_idx = LE_ORDER_ATTACK;
			break;
		case AI_GOAL_DOCK:
			eh_idx = LE_ORDER_DOCK;
			break;
		case AI_GOAL_WAYPOINTS:
			eh_idx = LE_ORDER_WAYPOINTS;
			break;
		case AI_GOAL_WAYPOINTS_ONCE:
			eh_idx = LE_ORDER_WAYPOINTS_ONCE;
			break;
		case AI_GOAL_WARP:
			eh_idx = LE_ORDER_DEPART;
			break;
		case AI_GOAL_FORM_ON_WING:
			eh_idx = LE_ORDER_FORM_ON_WING;
			break;
		case AI_GOAL_UNDOCK:
			eh_idx = LE_ORDER_UNDOCK;
			break;
		case AI_GOAL_GUARD:
			eh_idx = LE_ORDER_GUARD;
			break;
		case AI_GOAL_DISABLE_SHIP:
			eh_idx = LE_ORDER_DISABLE;
			break;
		case AI_GOAL_DISABLE_SHIP_TACTICAL:
			eh_idx = LE_ORDER_DISABLE_TACTICAL;
			break;
		case AI_GOAL_DISARM_SHIP:
			eh_idx = LE_ORDER_DISARM;
			break;
		case AI_GOAL_DISARM_SHIP_TACTICAL:
			eh_idx = LE_ORDER_DISARM_TACTICAL;
			break;
		case AI_GOAL_CHASE_ANY:
			eh_idx = LE_ORDER_ATTACK_ANY;
			break;
		case AI_GOAL_IGNORE:
			eh_idx = LE_ORDER_IGNORE;
			break;
		case AI_GOAL_IGNORE_NEW:
			eh_idx = LE_ORDER_IGNORE_NEW;
			break;
		case AI_GOAL_EVADE_SHIP:
			eh_idx = LE_ORDER_EVADE;
			break;
		case AI_GOAL_STAY_NEAR_SHIP:
			eh_idx = LE_ORDER_STAY_NEAR;
			break;
		case AI_GOAL_KEEP_SAFE_DISTANCE:
			eh_idx = LE_ORDER_KEEP_SAFE_DISTANCE;
			break;
		case AI_GOAL_REARM_REPAIR:
			eh_idx = LE_ORDER_REARM;
			break;
		case AI_GOAL_STAY_STILL:
			eh_idx = LE_ORDER_STAY_STILL;
			break;
		case AI_GOAL_PLAY_DEAD:
			eh_idx = LE_ORDER_PLAY_DEAD;
			break;
		case AI_GOAL_PLAY_DEAD_PERSISTENT:
			eh_idx = LE_ORDER_PLAY_DEAD_PERSISTENT;
			break;
		case AI_GOAL_FLY_TO_SHIP:
			eh_idx = LE_ORDER_FLY_TO;
			break;
		case AI_GOAL_CHASE_WING:
			eh_idx = LE_ORDER_ATTACK_WING;
			break;
		case AI_GOAL_GUARD_WING:
			eh_idx = LE_ORDER_GUARD_WING;
			break;
		case AI_GOAL_CHASE_SHIP_CLASS:
			eh_idx = LE_ORDER_ATTACK_SHIP_CLASS;
			break;
		case AI_GOAL_LUA:
			eh_idx = LE_ORDER_LUA;
			break;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
}

// helper function
void maybe_start_waypoints(order_h* ohp, bool force)
{
	Assertion(ohp->aigp->wp_list_index >= 0, "This function requires wp_list to be assigned already!");
	Assertion(ohp->aigp->target_name && !stricmp(Waypoint_lists[ohp->aigp->wp_list_index].get_name(), ohp->aigp->target_name), "wp_list name does not match target_name!");

	if (ohp->odx == 0)
	{
		int flags = 0;
		if (ohp->aigp->ai_mode == AI_GOAL_WAYPOINTS)
			flags |= WPF_REPEAT;
		if (ohp->aigp->flags[AI::Goal_Flags::Waypoints_in_reverse])
			flags |= WPF_BACKTRACK;
		ai_start_waypoints(ohp->objh.objp(), ohp->aigp->wp_list_index, flags, ohp->aigp->int_data, force);
	}
}

ADE_VIRTVAR(Target, l_Order, "object", "Target of the order. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Target object or invalid object handle if order handle is invalid or order requires no target.")
{
	order_h *ohp = NULL;
	object_h *newh = NULL;
	int shipnum = -1, wingnum = -1, objnum = -1;
	if(!ade_get_args(L, "o|o", l_Order.GetPtr(&ohp), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!ohp->isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	auto aip = &Ai_info[Ships[ohp->objh.objp()->instance].ai_index];

	if(ADE_SETTING_VAR){
		if(newh && newh->isValid()){
			switch(ohp->aigp->ai_mode){
				case AI_GOAL_DESTROY_SUBSYSTEM:
				case AI_GOAL_CHASE:
				case AI_GOAL_FORM_ON_WING:
				case AI_GOAL_GUARD:
				case AI_GOAL_DISABLE_SHIP:
				case AI_GOAL_DISABLE_SHIP_TACTICAL:
				case AI_GOAL_DISARM_SHIP:
				case AI_GOAL_DISARM_SHIP_TACTICAL:
				case AI_GOAL_IGNORE_NEW:
				case AI_GOAL_IGNORE:
				case AI_GOAL_EVADE_SHIP:
				case AI_GOAL_STAY_NEAR_SHIP:
				case AI_GOAL_FLY_TO_SHIP:
				case AI_GOAL_STAY_STILL:
					if ((newh->objp()->type == OBJ_SHIP) && stricmp(Ships[newh->objp()->instance].ship_name, ohp->aigp->target_name)) {
						ohp->aigp->target_name = ai_get_goal_target_name(Ships[newh->objp()->instance].ship_name, &ohp->aigp->target_name_index);
						ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;

						if(ohp->odx == 0) {
							aip->ok_to_target_timestamp = timestamp(0);
							set_target_objnum(aip, newh->objnum);
						}
					}
					break;

				case AI_GOAL_REARM_REPAIR:
				case AI_GOAL_DOCK:
				case AI_GOAL_UNDOCK:
					// TODO: these are currently not implemented, but could be implemented in the future
					break;

				case AI_GOAL_CHASE_WEAPON:
					if ((newh->objp()->type == OBJ_WEAPON) && (ohp->aigp->target_signature != newh->sig)){
						ohp->aigp->target_instance = newh->objp()->instance;
						ohp->aigp->target_signature = Weapons[newh->objp()->instance].objnum;
						ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;

						if(ohp->odx == 0) {
							aip->ok_to_target_timestamp = timestamp(0);
							set_target_objnum(aip, newh->objnum);
						}
					}
					break;

				case AI_GOAL_CHASE_SHIP_CLASS:
					// we can set the ship class from the class of the target
					if (newh->objp()->type == OBJ_SHIP) {
						auto class_name = Ship_info[Ships[newh->objp()->instance].ship_info_index].name;
						if (stricmp(class_name, ohp->aigp->target_name)) {
							ohp->aigp->target_name = ai_get_goal_target_name(class_name, &ohp->aigp->target_name_index);
							ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;

							if (ohp->odx == 0) {
								aip->ok_to_target_timestamp = timestamp(0);
								set_target_objnum(aip, newh->objnum);
							}
						}
					}
					break;

				case AI_GOAL_WAYPOINTS:
				case AI_GOAL_WAYPOINTS_ONCE:
					if (newh->objp()->type == OBJ_WAYPOINT) {
						int wpl, idx;
						calc_waypoint_indexes(newh->objp()->instance, wpl, idx);
						if (stricmp(Waypoint_lists[wpl].get_name(), ohp->aigp->target_name) || idx != ohp->aigp->int_data) {
							ohp->aigp->target_name = ai_get_goal_target_name(Waypoint_lists[wpl].get_name(), &ohp->aigp->target_name_index);
							ohp->aigp->wp_list_index = wpl;
							ohp->aigp->int_data = idx;
							ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;
							maybe_start_waypoints(ohp, true);
						}
					}
					break;

				case AI_GOAL_CHASE_WING:
				case AI_GOAL_GUARD_WING:
					if ((newh->objp()->type == OBJ_SHIP) && stricmp(Ships[newh->objp()->instance].ship_name, ohp->aigp->target_name)) {
						ship *shipp = &Ships[newh->objp()->instance];
						if (shipp->wingnum != -1){
							ohp->aigp->target_name = ai_get_goal_target_name(Wings[shipp->wingnum].name, &ohp->aigp->target_name_index);
							ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;

							if(ohp->odx == 0) {
								aip->ok_to_target_timestamp = timestamp(0);

								if (ohp->aigp->ai_mode == AI_GOAL_CHASE_WING)
									ai_attack_wing(ohp->objh.objp(),shipp->wingnum);
								else // AI_GOAL_GUARD_WING
									ai_set_guard_wing(ohp->objh.objp(), shipp->wingnum);
							}
						}
					}
					break;

				default:
					break;
			}
		}
	}

	switch(ohp->aigp->ai_mode){
		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_CHASE:
		case AI_GOAL_FORM_ON_WING:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISABLE_SHIP_TACTICAL:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISARM_SHIP_TACTICAL:
		case AI_GOAL_IGNORE_NEW:
		case AI_GOAL_IGNORE:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_FLY_TO_SHIP:
		case AI_GOAL_STAY_STILL:
		case AI_GOAL_REARM_REPAIR:
		case AI_GOAL_DOCK:
		case AI_GOAL_UNDOCK:
			shipnum = ship_name_lookup(ohp->aigp->target_name);
			objnum = (shipnum >= 0) ? Ships[shipnum].objnum : -1;
			break;

		case AI_GOAL_CHASE_WEAPON:
			objnum = Weapons[ohp->aigp->target_instance].objnum;
			break;

		case AI_GOAL_CHASE_SHIP_CLASS:
			// a ship class isn't an in-mission object
			return ade_set_args(L, "o", l_Object.Set(object_h()));

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			// check if waypoint order is the current goal (ohp->odx == 0) and if it is valid
			waypoint* wp;
			waypoint_list* wpl;
			if ( (ohp->odx == 0) && (aip->wp_index != INVALID_WAYPOINT_POSITION) && ((wpl = find_waypoint_list_at_index(aip->wp_list_index)) != nullptr) && ((wp = find_waypoint_at_index(wpl, aip->wp_index)) != nullptr) ) {
				objnum = wp->get_objnum();
			} else {
				wpl = find_matching_waypoint_list(ohp->aigp->target_name);
				if (wpl != nullptr) {
					objnum = wpl->get_waypoints()[ohp->aigp->int_data].get_objnum();
				}
			}
			break;

		case AI_GOAL_CHASE_WING:
		case AI_GOAL_GUARD_WING:
			wingnum = wing_name_lookup(ohp->aigp->target_name);
			if (Wings[wingnum].current_count > 0){
				shipnum = Wings[wingnum].ship_index[0];
				objnum = Ships[shipnum].objnum;
			}
			break;

		default:
			break;
	}

	return ade_set_object_with_breed(L, objnum);
}

ADE_VIRTVAR(TargetSubsystem, l_Order, "subsystem", "Target subsystem of the order.", "subsystem", "Target subsystem, or invalid subsystem handle if order handle is invalid or order requires no subsystem target.")
{
	order_h *ohp = NULL;
	ship_subsys_h *newh = NULL;
	ai_info *aip = NULL;
	object *objp = NULL;
	if(!ade_get_args(L, "o|o", l_Order.GetPtr(&ohp), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!ohp->isValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	aip = &Ai_info[Ships[ohp->objh.objp()->instance].ai_index];

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isValid() && (ohp->aigp->ai_mode == AI_GOAL_DESTROY_SUBSYSTEM))
		{
			objp = &Objects[newh->ss->parent_objnum];
			if (stricmp(Ships[objp->instance].ship_name, ohp->aigp->target_name)) {
				ohp->aigp->target_name = ai_get_goal_target_name(Ships[objp->instance].ship_name, &ohp->aigp->target_name_index);
				ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;
				if(ohp->odx == 0) {
					aip->ok_to_target_timestamp = timestamp(0);
					set_target_objnum(aip, OBJ_INDEX(objp));
				}
			}
			ohp->aigp->ai_submode = ship_find_subsys( &Ships[objp->instance], newh->ss->system_info->subobj_name );
			if(ohp->odx == 0) {
				set_targeted_subsys(aip, newh->ss, OBJ_INDEX(objp));
			}
			if (aip == Player_ai) {
				Ships[newh->ss->parent_objnum].last_targeted_subobject[Player_num] = newh->ss;
			}
		}
	}

	if(ohp->aigp->ai_mode == AI_GOAL_DESTROY_SUBSYSTEM){
		return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[Ships[ship_name_lookup(ohp->aigp->target_name)].objnum], ship_get_indexed_subsys(&Ships[ship_name_lookup(ohp->aigp->target_name)],ohp->aigp->ai_submode))));
	} else {
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));
	}
}

ADE_VIRTVAR(WaypointList, l_Order, "waypointlist", "Waypoint list of the order.", "waypointlist", "Waypoint list, or invalid handle if order handle is invalid or if this is not a waypoints order.")
{
	order_h* ohp = nullptr;
	waypointlist_h *wplh = nullptr;
	if (!ade_get_args(L, "o|o", l_Order.GetPtr(&ohp), l_WaypointList.GetPtr(&wplh)))
		return ADE_RETURN_NIL;

	if (!ohp->isValid() || (ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS && ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS_ONCE))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && wplh->isValid())
	{
		ohp->aigp->target_name = ai_get_goal_target_name(wplh->getList()->get_name(), &ohp->aigp->target_name_index);
		ohp->aigp->wp_list_index = wplh->wl_index;
		ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;

		if (!SCP_vector_inbounds(Waypoint_lists[ohp->aigp->wp_list_index].get_waypoints(), ohp->aigp->int_data))
		{
			if (ohp->aigp->flags[AI::Goal_Flags::Waypoints_in_reverse])
				ohp->aigp->int_data = static_cast<int>(Waypoint_lists[ohp->aigp->wp_list_index].get_waypoints().size() - 1);
			else
				ohp->aigp->int_data = 0;
		}

		maybe_start_waypoints(ohp, true);
	}

	return ade_set_args(L, "o", l_WaypointList.Set(waypointlist_h(ohp->aigp->wp_list_index)));
}

ADE_VIRTVAR(WaypointIndex, l_Order, "number", "Waypoint index of the order.", "number", "Waypoint index, or invalid 0 if order handle is invalid or if this is not a waypoints order.")
{
	order_h* ohp = nullptr;
	int index = 0;
	if (!ade_get_args(L, "o|i", l_Order.GetPtr(&ohp), &index))
		return ade_set_error(L, "i", 0);

	if (!ohp->isValid() || (ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS && ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS_ONCE))
		return ade_set_error(L, "i", 0);

	if (ADE_SETTING_VAR)
	{
		index--;	// Lua->FS2
		if (SCP_vector_inbounds(Waypoint_lists[ohp->aigp->wp_list_index].get_waypoints(), index))
		{
			ohp->aigp->int_data = index;
			ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;
			maybe_start_waypoints(ohp, true);
		}
	}

	return ade_set_args(L, "i", ohp->aigp->int_data + 1);	// FS2->Lua
}

ADE_VIRTVAR(WaypointsInReverse, l_Order, "boolean", "Waypoint-reverse flag of the order.", "boolean", "Waypoint-reverse flag, or invalid false if order handle is invalid or if this is not a waypoints order.")
{
	order_h* ohp = nullptr;
	bool reverse = false;
	if (!ade_get_args(L, "o|b", l_Order.GetPtr(&ohp), &reverse))
		return ade_set_error(L, "b", false);

	if (!ohp->isValid() || (ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS && ohp->aigp->ai_mode != AI_GOAL_WAYPOINTS_ONCE))
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
	{
		ohp->aigp->flags.set(AI::Goal_Flags::Waypoints_in_reverse, reverse);
		ohp->aigp->time = (ohp->odx == 0) ? Missiontime : 0;
		maybe_start_waypoints(ohp, true);
	}

	return ade_set_args(L, "b", ohp->aigp->flags[AI::Goal_Flags::Waypoints_in_reverse]);
}

ADE_FUNC(isValid, l_Order, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	order_h *ohp = NULL;
	if(!ade_get_args(L, "o", l_Order.GetPtr(&ohp)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", ohp->isValid());
}

//**********HANDLE: shiporders
ADE_OBJ(l_ShipOrders, object_h, "shiporders", "Ship orders");

ADE_FUNC(__len, l_ShipOrders, NULL, "Number of ship orders", "number", "Number of ship orders, or 0 if handle is invalid")
{
	object_h *objh = NULL;
	if(!ade_get_args(L, "o", l_ShipOrders.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid() || objh->objp()->type != OBJ_SHIP || Ships[objh->objp()->instance].ai_index < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ai_goal_num(&Ai_info[Ships[objh->objp()->instance].ai_index].goals[0]));
}

ADE_INDEXER(l_ShipOrders, "number Index", "Array of ship orders", "order", "Order, or invalid order handle on failure")
{
	object_h *objh = NULL;
	int i;

	if (!ade_get_args(L, "oi", l_ShipOrders.GetPtr(&objh), &i))
		return ade_set_error(L, "o", l_Order.Set(order_h()));

	i--; //Lua->FS2

	if (!objh->isValid() || i < 0 || i >= MAX_AI_GOALS)
		return ade_set_error(L, "o", l_Order.Set(order_h()));

	ai_info *aip = &Ai_info[Ships[objh->objp()->instance].ai_index];

	if (aip->goals[i].ai_mode != AI_GOAL_NONE)
		return ade_set_args(L, "o", l_Order.Set(order_h(objh->objp(), i)));
	else
		return ade_set_args(L, "o", l_Order.Set(order_h()));
}

ADE_FUNC(isValid, l_ShipOrders, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_ShipOrders.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", oh->isValid());
}


}
}


//
//

#include "waypoint.h"
#include "object.h"

namespace scripting {
namespace api {

//**********HANDLE: Waypoint
ADE_OBJ_DERIV(l_Waypoint, object_h, "waypoint", "waypoint handle", l_Object);

ADE_FUNC(getList, l_Waypoint, NULL, "Returns the waypoint list", "waypointlist", "waypointlist handle or invalid handle if waypoint was invalid")
{
	object_h *oh = NULL;
	waypoint_list *wp_list = NULL;
	if(!ade_get_args(L, "o", l_Waypoint.GetPtr(&oh)))
		return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));

	if(oh->isValid() && oh->objp()->type == OBJ_WAYPOINT) {
		wp_list = find_waypoint_list_with_instance(oh->objp()->instance);
		return ade_set_args(L, "o", l_WaypointList.Set(waypointlist_h(wp_list)));
	}

	return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));
}

ADE_FUNC(getIndex, l_Waypoint, nullptr, "Returns the index of this waypoint in its list", "number", "waypoint index or 0 if waypoint was invalid")
{
	object_h *oh = nullptr;
	if(!ade_get_args(L, "o", l_Waypoint.GetPtr(&oh)))
		return ade_set_error(L, "i", 0);

	if(!oh->isValid() || oh->objp()->type != OBJ_WAYPOINT)
		return ade_set_error(L, "i", 0);

	int wp_index = calc_waypoint_index(oh->objp()->instance);
	return ade_set_args(L, "i", wp_index + 1);
}

waypointlist_h::waypointlist_h()
	: wl_index(-1)
{}
waypointlist_h::waypointlist_h(int _wl_index)
{
	if (SCP_vector_inbounds(Waypoint_lists, _wl_index))
		wl_index = _wl_index;
	else
		wl_index = -1;
}
waypointlist_h::waypointlist_h(waypoint_list* _wlp)
	: waypointlist_h((_wlp == nullptr) ? -1 : find_index_of_waypoint_list(_wlp))
{}
waypointlist_h::waypointlist_h(const char* wlname)
	: waypointlist_h((wlname == nullptr) ? nullptr : find_matching_waypoint_list(wlname))
{}
bool waypointlist_h::isValid() const
{
	return SCP_vector_inbounds(Waypoint_lists, wl_index);
}
waypoint_list* waypointlist_h::getList() const
{
	return isValid() ? &Waypoint_lists[wl_index] : nullptr;
}

//**********HANDLE: WaypointList
ADE_OBJ(l_WaypointList, waypointlist_h, "waypointlist", "waypointlist handle");

ADE_INDEXER(l_WaypointList, "number Index", "Array of waypoints that are part of the waypoint list", "waypoint", "Waypoint, or invalid handle if the index or waypointlist handle is invalid")
{
	int idx = -1;
	waypointlist_h* wlh = nullptr;
	if( !ade_get_args(L, "oi", l_WaypointList.GetPtr( &wlh ), &idx))
		return ade_set_error( L, "o", l_Waypoint.Set( object_h() ) );

	if(!wlh || !wlh->isValid())
		return ade_set_error( L, "o", l_Waypoint.Set( object_h() ) );

	//Lua-->FS2
	idx--;

	//Get waypoint
	auto wp = find_waypoint_at_index(wlh->getList(), idx);
	if (wp)
		return ade_set_args(L, "o", l_Waypoint.Set(object_h(wp->get_objnum())));

	return ade_set_error(L, "o", l_Waypoint.Set( object_h() ) );
}

ADE_FUNC(__len, l_WaypointList,
		 NULL,
		 "Number of waypoints in the list. "
			 "Note that the value returned cannot be relied on for more than one frame.",
		 "number",
		 "Number of waypoints in the list, or 0 if handle is invalid")
{
	waypointlist_h* wlh = nullptr;
	if ( !ade_get_args(L, "o", l_WaypointList.GetPtr(&wlh)) ) {
		return ade_set_error(L, "i", 0);
	}
	if (!wlh || !wlh->isValid()) {
		return ade_set_error(L, "i", 0);
	}

	return ade_set_args(L, "i", wlh->getList()->get_waypoints().size());
}

ADE_FUNC(__tostring, l_WaypointList, nullptr, "Returns name of waypoint list (if any)", "string", "Waypoint list name, or empty string if handle is invalid")
{
	waypointlist_h* wlh = nullptr;
	if ( !ade_get_args(L, "o", l_WaypointList.GetPtr(&wlh)) ) {
		return ade_set_error(L, "s", "");
	}
	if (!wlh || !wlh->isValid()) {
		return ade_set_error(L, "s", "");
	}

	return ade_set_args(L, "s", wlh->getList()->get_name());
}

ADE_VIRTVAR(Name, l_WaypointList, "string", "Name of WaypointList", "string", "Waypointlist name, or empty string if handle is invalid")
{
	waypointlist_h* wlh = NULL;
	const char* s       = nullptr;
	if ( !ade_get_args(L, "o|s", l_WaypointList.GetPtr(&wlh), &s) ) {
		return ade_set_error(L, "s", "");
	}
	if (!wlh || !wlh->isValid()) {
		return ade_set_error(L, "s", "");
	}

	if(ADE_SETTING_VAR && s != NULL) {
		wlh->getList()->set_name(s);
	}

	return ade_set_args( L, "s", wlh->getList()->get_name());
}

ADE_FUNC(isValid, l_WaypointList, NULL, "Return if this waypointlist handle is valid", "boolean", "true if valid false otherwise")
{
	waypointlist_h* wlh = NULL;
	if ( !ade_get_args(L, "o", l_WaypointList.GetPtr(&wlh)) ) {
		return ADE_RETURN_FALSE;
	}
	return ade_set_args(L, "b", wlh != NULL && wlh->isValid());
}

}
}

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
	waypointlist_h wpl;
	waypoint_list *wp_list = NULL;
	if(!ade_get_args(L, "o", l_Waypoint.GetPtr(&oh)))
		return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));

	if(oh->IsValid() && oh->objp->type == OBJ_WAYPOINT) {
		wp_list = find_waypoint_list_with_instance(oh->objp->instance);
		if(wp_list != NULL)
			wpl = waypointlist_h(wp_list);
	}

	if (wpl.IsValid()) {
		return ade_set_args(L, "o", l_WaypointList.Set(wpl));
	}

	return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));
}

waypointlist_h::waypointlist_h() {wlp=NULL;name[0]='\0';}
waypointlist_h::waypointlist_h(waypoint_list* n_wlp) {
	wlp = n_wlp;
	if(n_wlp != NULL) {
		strcpy_s(name, wlp->get_name());
	} else {
		memset(name, 0, sizeof(name));
	}
}
waypointlist_h::waypointlist_h(char* wlname) {
	wlp = NULL;
	if ( wlname != NULL ) {
		strcpy_s(name, wlname);
		wlp = find_matching_waypoint_list(wlname);
	}
}
bool waypointlist_h::IsValid() {
	return (wlp != NULL && !strcmp(wlp->get_name(), name));
}

//**********HANDLE: WaypointList
ADE_OBJ(l_WaypointList, waypointlist_h, "waypointlist", "waypointlist handle");

ADE_INDEXER(l_WaypointList, "number Index", "Array of waypoints that are part of the waypoint list", "waypoint", "Waypoint, or invalid handle if the index or waypointlist handle is invalid")
{
	int idx = -1;
	waypointlist_h* wlh = NULL;
	char wpname[128];
	if( !ade_get_args(L, "oi", l_WaypointList.GetPtr( &wlh ), &idx))
		return ade_set_error( L, "o", l_Waypoint.Set( object_h() ) );

	if(!wlh->IsValid())
		return ade_set_error( L, "o", l_Waypoint.Set( object_h() ) );

	//Lua-->FS2
	idx--;

	//Get waypoint name
	sprintf(wpname, "%s:%d", wlh->wlp->get_name(), calc_waypoint_index(idx) + 1);
	waypoint *wpt = find_matching_waypoint( wpname );
	if( (idx >= 0) && ((uint) idx < wlh->wlp->get_waypoints().size()) && (wpt != NULL) && (wpt->get_objnum() >= 0) ) {
		return ade_set_args( L, "o", l_Waypoint.Set( object_h( &Objects[wpt->get_objnum()] ), Objects[wpt->get_objnum()].signature ) );
	}

	return ade_set_error(L, "o", l_Waypoint.Set( object_h() ) );
}

ADE_FUNC(__len, l_WaypointList,
		 NULL,
		 "Number of waypoints in the list. "
			 "Note that the value returned cannot be relied on for more than one frame.",
		 "number",
		 "Number of waypoints in the list, or 0 if handle is invalid")
{
	waypointlist_h* wlh = NULL;
	if ( !ade_get_args(L, "o", l_WaypointList.GetPtr(&wlh)) ) {
		return ade_set_error( L, "o", l_Waypoint.Set( object_h() ) );
	}
	return ade_set_args(L, "i", wlh->wlp->get_waypoints().size());
}

ADE_VIRTVAR(Name, l_WaypointList, "string", "Name of WaypointList", "string", "Waypointlist name, or empty string if handle is invalid")
{
	waypointlist_h* wlh = NULL;
	char *s = NULL;
	if ( !ade_get_args(L, "o|s", l_WaypointList.GetPtr(&wlh), &s) ) {
		return ade_set_error(L, "s", "");
	}

	if(ADE_SETTING_VAR && s != NULL) {
		wlh->wlp->set_name(s);
		strcpy_s(wlh->name,s);
	}

	return ade_set_args( L, "s", wlh->name);
}

ADE_FUNC(isValid, l_WaypointList, NULL, "Return if this waypointlist handle is valid", "boolean", "true if valid false otherwise")
{
	waypointlist_h* wlh = NULL;
	if ( !ade_get_args(L, "o", l_WaypointList.GetPtr(&wlh)) ) {
		return ADE_RETURN_FALSE;
	}
	return ade_set_args(L, "b", wlh != NULL && wlh->IsValid());
}

}
}

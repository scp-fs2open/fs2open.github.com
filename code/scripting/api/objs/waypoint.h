#pragma once

#include "scripting/ade_api.h"

#include "object/object.h"
#include "object/waypoint.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ(l_Waypoint, object_h);

struct waypointlist_h
{
	waypoint_list *wlp;
	char name[NAME_LENGTH];
	waypointlist_h();
	explicit waypointlist_h(waypoint_list *n_wlp);
	explicit waypointlist_h(char wlname[NAME_LENGTH]);
	bool IsValid();
};

DECLARE_ADE_OBJ(l_WaypointList, waypointlist_h);


}
}


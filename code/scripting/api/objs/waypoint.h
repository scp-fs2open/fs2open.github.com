#pragma once

#include "scripting/ade_api.h"

#include "object/object.h"
#include "object/waypoint.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ(l_Waypoint, object_h);

struct waypointlist_h
{
	int wl_index;
	waypointlist_h();
	explicit waypointlist_h(int _wl_index);
	explicit waypointlist_h(waypoint_list* _wlp);
	explicit waypointlist_h(const char* wlname);
	bool isValid() const;
	waypoint_list* getList() const;
};

DECLARE_ADE_OBJ(l_WaypointList, waypointlist_h);


}
}


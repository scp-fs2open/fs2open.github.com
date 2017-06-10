
#include "mission/management.h"

#include "object.h"

#include <object/waypoint.h>
#include <object/object.h>
#include <ship/ship.h>

namespace fso {
namespace fred {


const char* object_name(int obj) {
	static char text[80];
	waypoint_list *wp_list;
	int waypoint_num;

	if (!query_valid_object(obj))
		return "*none*";

	switch (Objects[obj].type) {
	case OBJ_SHIP:
	case OBJ_START:
		return Ships[Objects[obj].instance].ship_name;

	case OBJ_WAYPOINT:
		wp_list = find_waypoint_list_with_instance(Objects[obj].instance, &waypoint_num);
		Assert(wp_list != NULL);
		sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);
		return text;

	case OBJ_POINT:
		return "Briefing icon";
	}

	return "*unknown*";
}


}
}

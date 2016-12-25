#pragma once

#include "scripting/ade_api.h"
#include "object/object.h"
#include "ship/ship.h"

namespace scripting {
namespace api {


struct ship_subsys_h : public object_h
{
	ship_subsys *ss;	//Pointer to subsystem, or NULL for the hull
	ship_subsys_h();
	ship_subsys_h(object *objp_in, ship_subsys *sub);

	bool IsValid();
};
DECLARE_ADE_OBJ(l_Subsystem, ship_subsys_h);

}
}


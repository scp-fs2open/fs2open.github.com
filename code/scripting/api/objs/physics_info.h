//
//

#ifndef FS2_OPEN_PHYSICS_INFO_H
#define FS2_OPEN_PHYSICS_INFO_H

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include "object/object.h"
#include "physics/physics.h"

namespace scripting {
namespace api {

//**********HANDLE: physics
struct physics_info_h
{
	object_h objh;
	physics_info *pi;

	physics_info_h();

	explicit physics_info_h(object *objp);

	physics_info_h(physics_info *in_pi);

	bool IsValid();
};

DECLARE_ADE_OBJ(l_Physics, physics_info_h);

}
}

#endif //FS2_OPEN_PHYSICS_INFO_H

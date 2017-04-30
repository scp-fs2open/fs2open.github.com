#pragma once

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include "model.h"
#include "object/object.h"
#include "model/model.h"

namespace scripting {
namespace api {

const int THT_INDEPENDENT	= 0;
const int THT_OBJECT			= 1;
const int THT_MODEL			= 2;

class texture_map_h
{
 protected:
	int type;
	object_h obj;
	model_h mdl;

	texture_map *tmap;	//Pointer to subsystem, or NULL for the hull

 public:
	texture_map_h();

	texture_map_h(object *objp, texture_map *n_tmap = NULL);

	texture_map_h(int modelnum, texture_map *n_tmap = NULL);

	texture_map_h(polymodel *n_model, texture_map *n_tmap = NULL);

	texture_map *Get();

	int GetSize();

	bool IsValid();
};

DECLARE_ADE_OBJ(l_TextureMap, texture_map_h);

}
}


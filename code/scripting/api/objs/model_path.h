#pragma once

#include "scripting/ade_api.h"

#include "model/model.h"

#include "subsystem.h"

namespace scripting {
namespace api {

struct model_path_h {
	ship_subsys_h subsys;

	SCP_string name; // name of the subsystem.
	int parent_submodel;
	SCP_vector<mp_vert> verts;

	model_path_h();

	model_path_h(const ship_subsys_h& _subsys, const model_path& _path);

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_ModelPath, model_path_h);

} // namespace api
} // namespace scripting

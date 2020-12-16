//
//

#include "model_path.h"
#include "vecmath.h"

namespace scripting {
namespace api {

struct model_path_point_h {
	const model_path_h* parent = nullptr;

	vec3d point{};
	float radius = -1.0f;

	model_path_point_h() = default;

	model_path_point_h(const model_path_h* _parent, const vec3d& _point, float _radius)
	    : parent(_parent), point(_point), radius(_radius)
	{
	}

	bool isValid() const { return parent != nullptr && radius > 0.0f; }
};

ADE_OBJ(l_ModelPathPoint, model_path_point_h, "modelpathpoint", "Point in a model path");

ADE_FUNC(isValid, l_ModelPathPoint, nullptr, "Determines if the handle is valid", "boolean",
         "True if valid, false otherwise")
{
	model_path_point_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPathPoint.GetPtr(&p)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", p->isValid());
}

ADE_VIRTVAR(Position, l_ModelPathPoint, "vector", "The current, global position of this path point.", "vector",
            "The current position of the point.")
{
	model_path_point_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPathPoint.GetPtr(&p)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!p->isValid()) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	// A submodel is only valid if the object is a ship so this is safe
	auto objp  = p->parent->subsys.objp;
	auto shipp = &Ships[objp->instance];

	auto pmi = model_get_instance(shipp->model_instance_num);
	auto pm = model_get(pmi->model_num);

	vec3d world_pos;
	model_instance_find_world_point(&world_pos, &p->point, pm, pmi, 0, &objp->orient, &objp->pos);

	return ade_set_error(L, "o", l_Vector.Set(world_pos));
}
ADE_VIRTVAR(Radius, l_ModelPathPoint, "number", "The radius of the path point.", "number", "The radius of the point.")
{
	model_path_point_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPathPoint.GetPtr(&p)))
		return ade_set_error(L, "f", -1.0f);

	if (!p->isValid()) {
		return ade_set_error(L, "f", -1.0f);
	}

	return ade_set_error(L, "f", p->radius);
}

model_path_h::model_path_h() = default;
model_path_h::model_path_h(const ship_subsys_h& _subsys, const model_path& _path) : subsys(_subsys)
{
	name            = _path.name;
	parent_submodel = _path.parent_submodel;

	verts.reserve(_path.nverts);
	for (auto i = 0; i < _path.nverts; ++i) {
		verts.push_back(_path.verts[i]);
	}
}
bool model_path_h::isValid() const { return subsys.isSubsystemValid(); }

ADE_OBJ(l_ModelPath, model_path_h, "modelpath", "Path of a model");

ADE_FUNC(__len, l_ModelPath, nullptr, "Gets the number of points in this path", "number",
         "The number of points or 0 on error")
{
	model_path_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPath.GetPtr(&p)))
		return ade_set_error(L, "s", "");

	if (p == nullptr)
		return ade_set_error(L, "s", "");

	if (!p->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "i", static_cast<int>(p->verts.size()));
}

ADE_FUNC(isValid, l_ModelPath, nullptr, "Determines if the handle is valid", "boolean",
         "True if valid, false otherwise")
{
	model_path_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPath.GetPtr(&p)))
		return ADE_RETURN_FALSE;

	if (p == nullptr)
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", p->isValid());
}

ADE_INDEXER(l_ModelPath, "number", "Returns the point in the path with the specified index", "modelpathpoint",
            "The point or invalid handle if index is invalid")
{
	model_path_h* p = nullptr;
	int idx;
	if (!ade_get_args(L, "oi", l_ModelPath.GetPtr(&p), &idx))
		return ade_set_error(L, "o", l_ModelPathPoint.Set(model_path_point_h()));

	if (p == nullptr)
		return ade_set_error(L, "o", l_ModelPathPoint.Set(model_path_point_h()));

	if (!p->isValid())
		return ade_set_error(L, "o", l_ModelPathPoint.Set(model_path_point_h()));

	idx = idx - 1; // Lua -> FS2

	if (idx < 0 || idx >= (int)p->verts.size()) {
		return ade_set_error(L, "o", l_ModelPathPoint.Set(model_path_point_h()));
	}

	auto& vert = p->verts[idx];

	return ade_set_args(L, "o", l_ModelPathPoint.Set(model_path_point_h(p, vert.pos, vert.radius)));
}

ADE_VIRTVAR(Name, l_ModelPath, "string", "The name of this model path", "string",
            "The name or empty string if handle is invalid")
{
	model_path_h* p = nullptr;
	if (!ade_get_args(L, "o", l_ModelPath.GetPtr(&p)))
		return ade_set_error(L, "s", "");

	if (p == nullptr)
		return ade_set_error(L, "s", "");

	if (!p->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", p->name.c_str());
}

} // namespace api
} // namespace scripting

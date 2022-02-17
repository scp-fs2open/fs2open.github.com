//
//

#include "mc_info.h"
#include "model.h"
#include "vecmath.h"

namespace scripting {
namespace api {

mc_info_h::mc_info_h(const mc_info& val) : info(val), valid(true) {}

mc_info_h::mc_info_h() = default;

mc_info* mc_info_h::Get() { return &info; }
bool mc_info_h::IsValid() { return valid; }

//**********HANDLE: Collision info
ADE_OBJ(l_ColInfo, mc_info_h, "collision_info", "Information about a collision");

ADE_VIRTVAR(Model, l_ColInfo, "model", "The model this collision info is about", "model", "The model, or an invalid model if the handle is not valid")
{
	mc_info_h* info;
	model_h * mh = nullptr;

	if(!ade_get_args(L, "o|o", l_ColInfo.GetPtr(&info), l_Model.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	if (!info->IsValid())
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	mc_info *collide = info->Get();

	int modelNum = collide->model_num;

	if (ADE_SETTING_VAR && mh)
	{
		if (mh->IsValid())
		{
			collide->model_num = mh->GetID();
		}
	}

	return ade_set_args(L, "o", l_Model.Set(model_h(modelNum)));
}

ADE_FUNC(getCollisionSubmodel, l_ColInfo, nullptr, "The submodel where the collision occurred, if applicable", "submodel", "The submodel, or nil if none or if the handle is not valid")
{
	mc_info_h *info;
	submodel_h *sh = nullptr;

	if (!ade_get_args(L, "o|o", l_ColInfo.GetPtr(&info), l_Submodel.GetPtr(&sh)))
		return ADE_RETURN_NIL;

	if (!info->IsValid())
		return ADE_RETURN_NIL;

	mc_info *collide = info->Get();

	if (collide->hit_submodel < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(collide->model_num, collide->hit_submodel)));
}

ADE_FUNC(getCollisionDistance, l_ColInfo, NULL, "The distance to the closest collision point", "number", "distance or -1 on error")
{
	mc_info_h* info;

	if(!ade_get_args(L, "o", l_ColInfo.GetPtr(&info)))
		return ade_set_error(L, "f", -1.0f);

	if (!info->IsValid())
		return ade_set_error(L, "f", -1.0f);

	mc_info *collide = info->Get();

	if (collide->num_hits <= 0)
	{
		return ade_set_args(L, "f", -1.0f);;
	}
	else
	{
		return ade_set_args(L, "f", collide->hit_dist);
	}
}

ADE_FUNC(getCollisionPoint, l_ColInfo, "[boolean local]", "The collision point of this information (local to the object if boolean is set to <i>true</i>)", "vector", "The collision point, or nil if none or if the handle is not valid")
{
	mc_info_h* info;
	bool local = false;

	if(!ade_get_args(L, "o|b", l_ColInfo.GetPtr(&info), &local))
		return ADE_RETURN_NIL;

	if (!info->IsValid())
		return ADE_RETURN_NIL;

	mc_info *collide = info->Get();

	if (collide->num_hits <= 0)
	{
		return ADE_RETURN_NIL;
	}
	else
	{
		if (local)
			return ade_set_args(L, "o", l_Vector.Set(collide->hit_point));
		else
			return ade_set_args(L, "o", l_Vector.Set(collide->hit_point_world));
	}
}

ADE_FUNC(getCollisionNormal, l_ColInfo, "[boolean local]", "The collision normal of this information (local to object if boolean is set to <i>true</i>)", "vector", "The collision normal, or nil if none or if the handle is not valid")
{
	mc_info_h* info;
	bool local = false;

	if(!ade_get_args(L, "o|b", l_ColInfo.GetPtr(&info), &local))
		return ADE_RETURN_NIL;

	if (!info->IsValid())
		return ADE_RETURN_NIL;

	mc_info *collide = info->Get();

	if (collide->num_hits <= 0)
	{
		return ADE_RETURN_NIL;
	}
	else
	{
		if (!local)
		{
			vec3d normal;
			auto pmi = model_get_instance(collide->model_instance_num);
			auto pm = model_get(pmi->model_num);

			model_instance_local_to_global_dir(&normal, &collide->hit_normal, pm, pmi, collide->hit_submodel, collide->orient);

			return ade_set_args(L, "o", l_Vector.Set(normal));
		}
		else
		{
			return ade_set_args(L, "o", l_Vector.Set(collide->hit_normal));
		}
	}
}

ADE_FUNC(isValid, l_ColInfo, NULL, "Detects if this handle is valid", "boolean", "true if valid false otherwise")
{
	mc_info_h* info;

	if(!ade_get_args(L, "o", l_ColInfo.GetPtr(&info)))
		return ADE_RETURN_FALSE;

	if (info->IsValid())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

}
}

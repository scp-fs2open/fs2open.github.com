//
//

#include "mc_info.h"
#include "model.h"
#include "vecmath.h"

namespace scripting {
namespace api {

mc_info_h::mc_info_h(const mc_info& val) : info(val), valid(true) {}

mc_info_h::mc_info_h() : valid(false) {}

mc_info* mc_info_h::Get() { return &info; }
bool mc_info_h::IsValid() { return valid; }

//**********HANDLE: Collision info
ADE_OBJ(l_ColInfo, mc_info_h, "collision info", "Information about a collision");

ADE_VIRTVAR(Model, l_ColInfo, "model", "The model this collision info is about", "model", "The model")
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

ADE_FUNC(getCollisionPoint, l_ColInfo, "[boolean local]", "The collision point of this information (local to the object if boolean is set to <i>true</i>)", "vector", "The collision point or nil of none")
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

ADE_FUNC(getCollisionNormal, l_ColInfo, "[boolean local]", "The collision normal of this information (local to object if boolean is set to <i>true</i>)", "vector", "The collision normal or nil of none")
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

			vm_vec_unrotate(&normal, &collide->hit_normal, collide->orient);

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
		return ADE_RETURN_NIL;

	if (info->IsValid())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

}
}

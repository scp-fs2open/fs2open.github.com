//
//

#include "model.h"
#include "modelinstance.h"
#include "object.h"
#include "vecmath.h"

namespace scripting {
namespace api {

ADE_OBJ(l_ModelInstance, modelinstance_h, "model_instance", "Model instance handle");

modelinstance_h::modelinstance_h(int pmi_id)
{
	_pmi = model_get_instance(pmi_id);
}
modelinstance_h::modelinstance_h(polymodel_instance *pmi)
	: _pmi(pmi)
{}
modelinstance_h::modelinstance_h()
	: _pmi(nullptr)
{}
polymodel_instance *modelinstance_h::Get()
{
	return _pmi;
}
bool modelinstance_h::IsValid()
{
	return (_pmi != nullptr);
}


ADE_OBJ(l_SubmodelInstance, submodelinstance_h, "submodel_instance", "Submodel instance handle");

submodelinstance_h::submodelinstance_h(int pmi_id, int submodel_num)
	: _submodel_num(submodel_num)
{
	_pmi = model_get_instance(pmi_id);
	_pm = _pmi ? model_get(_pmi->model_num) : nullptr;
}
submodelinstance_h::submodelinstance_h(polymodel_instance *pmi, int submodel_num)
	: _pmi(pmi), _submodel_num(submodel_num)
{
	_pm = pmi ? model_get(pmi->model_num) : nullptr;
}
submodelinstance_h::submodelinstance_h()
	: _pmi(nullptr), _pm(nullptr), _submodel_num(-1)
{}
polymodel_instance *submodelinstance_h::GetModelInstance()
{
	return IsValid() ? _pmi : nullptr;
}
submodel_instance *submodelinstance_h::Get()
{
	return IsValid() ? &_pmi->submodel[_submodel_num] : nullptr;
}
polymodel *submodelinstance_h::GetModel()
{
	return IsValid() ? _pm : nullptr;
}
bsp_info *submodelinstance_h::GetSubmodel()
{
	return IsValid() ? &_pm->submodel[_submodel_num] : nullptr;
}
int submodelinstance_h::GetSubmodelIndex()
{
	return IsValid() ? _submodel_num : -1;
}
bool submodelinstance_h::IsValid()
{
	return _pmi != nullptr && _pm != nullptr && _submodel_num >= 0 && _submodel_num < _pm->n_models;
}


ADE_FUNC(getModel, l_ModelInstance, nullptr, "Returns the model used by this instance", "model", "A model")
{
	modelinstance_h *mih;
	if (!ade_get_args(L, "o", l_ModelInstance.GetPtr(&mih)))
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	return ade_set_args(L, "o", l_Model.Set(model_h(mih->Get()->model_num)));
}

ADE_FUNC(getObject, l_ModelInstance, nullptr, "Returns the object that this instance refers to", "object", "An object")
{
	modelinstance_h *mih;
	if (!ade_get_args(L, "o", l_ModelInstance.GetPtr(&mih)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (mih->Get()->objnum < 0)
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	return ade_set_object_with_breed(L, mih->Get()->objnum);
}

ADE_VIRTVAR(SubmodelInstances, l_ModelInstance, nullptr, "Submodel instances", "submodel_instances", "Model submodel instances, or an invalid modelsubmodelinstances handle if the model instance handle is invalid")
{
	modelinstance_h *mih = nullptr;
	if (!ade_get_args(L, "o", l_ModelInstance.GetPtr(&mih)))
		return ade_set_error(L, "o", l_ModelSubmodelInstances.Set(modelsubmodelinstances_h()));

	auto pmi = mih->Get();
	if (!pmi)
		return ade_set_error(L, "o", l_ModelSubmodelInstances.Set(modelsubmodelinstances_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Attempt to use Incomplete Feature: submodel instance copy");

	return ade_set_args(L, "o", l_ModelSubmodelInstances.Set(modelsubmodelinstances_h(pmi)));
}

ADE_FUNC(isValid, l_ModelInstance, nullptr, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	modelinstance_h *mih;
	if (!ade_get_args(L, "o", l_ModelInstance.GetPtr(&mih)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", mih->IsValid());
}

ADE_VIRTVAR(Orientation, l_SubmodelInstance, "orientation", "Gets or sets the submodel instance orientation", "orientation", "Orientation, or identity orientation if handle is not valid")
{
	submodelinstance_h *smih;
	matrix_h *mh = nullptr;
	if (!ade_get_args(L, "o|o", l_SubmodelInstance.GetPtr(&smih), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (!smih->IsValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	auto smi = smih->Get();
	if (smi == nullptr)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (ADE_SETTING_VAR && mh != nullptr)
	{
		smi->canonical_prev_orient = smi->canonical_orient;
		smi->canonical_orient = *mh->GetMatrix();

		float angle = 0.0f;
		vm_closest_angle_to_matrix(&smi->canonical_orient, &smih->GetSubmodel()->rotation_axis, &angle);

		smi->cur_angle = angle;
		smi->turret_idle_angle = angle;
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&smi->canonical_orient)));
}

ADE_VIRTVAR(TranslationOffset,
	l_SubmodelInstance,
	"vector",
	"Gets or sets the translated submodel instance offset.  This is relative to the existing submodel offset to its parent; a non-translated submodel will have a TranslationOffset of zero.",
	"vector",
	"Offset, or zero vector if handle is not valid")
{
	submodelinstance_h *smih;
	vec3d *vec = nullptr;
	if (!ade_get_args(L, "o|o", l_SubmodelInstance.GetPtr(&smih), l_Vector.GetPtr(&vec)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!smih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR && vec != nullptr)
	{
		auto smi = smih->Get();

		smi->canonical_prev_offset = smi->canonical_offset;
		smi->canonical_offset = *vec;

		smi->cur_offset = vm_vec_mag(vec);
	}

	return ade_set_args(L, "o", l_Vector.Set(smih->Get()->canonical_offset));
}

ADE_FUNC(findWorldPoint, l_SubmodelInstance, "vector", "Calculates the world coordinates of a point in a submodel's frame of reference", "vector", "Point, or empty vector if handle is not valid")
{
	submodelinstance_h *smih;
	vec3d pnt, outpnt;
	if (!ade_get_args(L, "oo", l_SubmodelInstance.GetPtr(&smih), l_Vector.Get(&pnt)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!smih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	auto pmi = smih->GetModelInstance();
	if (pmi->objnum < 0)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	auto objp = &Objects[pmi->objnum];

	model_instance_local_to_global_point(&outpnt, &pnt, pmi->id, smih->GetSubmodelIndex(), &objp->orient, &objp->pos);
	return ade_set_args(L, "o", l_Vector.Set(outpnt));
}

ADE_FUNC(findWorldDir, l_SubmodelInstance, "vector", "Calculates the world direction of a vector in a submodel's frame of reference", "vector", "Vector, or empty vector if handle is not valid")
{
	submodelinstance_h *smih;
	vec3d dir, outdir;
	if (!ade_get_args(L, "oo", l_SubmodelInstance.GetPtr(&smih), l_Vector.Get(&dir)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!smih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	auto pmi = smih->GetModelInstance();
	if (pmi->objnum < 0)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	auto objp = &Objects[pmi->objnum];

	model_instance_local_to_global_dir(&outdir, &dir, smih->GetModel(), pmi, smih->GetSubmodelIndex(), &objp->orient);
	return ade_set_args(L, "o", l_Vector.Set(outdir));
}

bool findObjectPointAndOrientationSub(lua_State *L, bool use_object, vec3d *outpnt, matrix *outorient)
{
	submodelinstance_h *smih;
	vec3d local_pnt;
	matrix_h *local_orient;
	if (!ade_get_args(L, "ooo", l_SubmodelInstance.GetPtr(&smih), l_Vector.Get(&local_pnt), l_Matrix.GetPtr(&local_orient)))
		return false;

	if (!smih->IsValid())
		return false;

	auto pmi = smih->GetModelInstance();
	if (pmi->objnum < 0)
		return false;
	auto objp = &Objects[pmi->objnum];

	model_instance_local_to_global_point_orient(outpnt, outorient, &local_pnt, local_orient->GetMatrix(), smih->GetModel(), pmi, smih->GetSubmodelIndex(), use_object ? &objp->orient : nullptr, use_object ? &objp->pos : nullptr);
	return true;
}

ADE_FUNC(findObjectPointAndOrientation, l_SubmodelInstance, "vector, orientation", "Calculates the coordinates and orientation, in an object's frame of reference, of a point and orientation in a submodel's frame of reference", "vector, orientation", "Vector and orientation, or empty values if a handle is invalid")
{
	vec3d pnt;
	matrix orient;

	if (!findObjectPointAndOrientationSub(L, false, &pnt, &orient))
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Matrix.Set(matrix_h(&vmd_zero_matrix)));

	return ade_set_args(L, "oo", l_Vector.Set(pnt), l_Matrix.Set(matrix_h(&orient)));
}

ADE_FUNC(findWorldPointAndOrientation, l_SubmodelInstance, "vector, orientation", "Calculates the world coordinates and orientation of a point and orientation in a submodel's frame of reference", "vector, orientation", "Vector and orientation, or empty values if a handle is invalid")
{
	vec3d pnt;
	matrix orient;

	if (!findObjectPointAndOrientationSub(L, true, &pnt, &orient))
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Matrix.Set(matrix_h(&vmd_zero_matrix)));

	return ade_set_args(L, "oo", l_Vector.Set(pnt), l_Matrix.Set(matrix_h(&orient)));
}

ADE_FUNC(isValid, l_SubmodelInstance, nullptr, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	submodelinstance_h *smih;
	if (!ade_get_args(L, "o", l_SubmodelInstance.GetPtr(&smih)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", smih->IsValid());
}


ADE_OBJ(l_ModelSubmodelInstances, modelsubmodelinstances_h, "submodel_instances", "Array of submodel instances");

modelsubmodelinstances_h::modelsubmodelinstances_h(polymodel_instance *pmi) : modelinstance_h(pmi){}
modelsubmodelinstances_h::modelsubmodelinstances_h() : modelinstance_h(){}

ADE_FUNC(__len, l_ModelSubmodelInstances, nullptr, "Number of submodel instances on model", "number", "Number of model submodel instances")
{
	modelsubmodelinstances_h *msih;
	if (!ade_get_args(L, "o", l_ModelSubmodelInstances.GetPtr(&msih)))
		return ade_set_error(L, "i", 0);

	if (!msih->IsValid())
		return ade_set_error(L, "i", 0);

	auto pmi = msih->Get();
	if (!pmi)
		return ade_set_error(L, "i", 0);
	auto pm = model_get(pmi->model_num);
	if (!pm)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_models);
}

ADE_INDEXER(l_ModelSubmodelInstances, "submodel_instance", "number|string IndexOrName", "submodel_instance", "Model submodel instances, or invalid modelsubmodelinstances handle if model instance handle is invalid")
{
	modelsubmodelinstances_h *msih = nullptr;
	int index = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi", l_ModelSubmodelInstances.GetPtr(&msih), &index))
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		if (!msih->IsValid())
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index--; // Lua --> C/C++
	}
	else if (lua_isstring(L, 2))
	{
		const char *name = nullptr;

		if (!ade_get_args(L, "os", l_ModelSubmodelInstances.GetPtr(&msih), &name))
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		if (!msih->IsValid() || name == nullptr)
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index = model_find_submodel_index(msih->Get()->model_num, name);
	}
	else
	{
		submodel_h *smh;

		if (!ade_get_args(L, "oo", l_ModelSubmodelInstances.GetPtr(&msih), l_Submodel.GetPtr(&smh)))
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		if (!msih->IsValid() || !smh->IsValid())
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index = smh->GetSubmodelIndex();
	}

	auto pmi = msih->Get();
	auto pm = model_get(pmi->model_num);

	if (index < 0 || index >= pm->n_models)
		return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

	return ade_set_args(L, "o", l_SubmodelInstance.Set(submodelinstance_h(pmi, index)));
}

ADE_FUNC(isValid, l_ModelSubmodelInstances, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if invalid, nil if a syntax/type error occurs")
{
	modelsubmodelinstances_h *msih;
	if (!ade_get_args(L, "o", l_ModelSubmodelInstances.GetPtr(&msih)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", msih->IsValid());
}

}
}

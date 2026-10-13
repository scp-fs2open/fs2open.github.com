//
//

#include "model.h"
#include "modelinstance.h"
#include "object.h"
#include "vecmath.h"
#include "texture.h"
#include "texturemap.h"

namespace scripting {
namespace api {

//**********HANDLE: modelinstancetextures (compatible with preceding shiptextures)
ADE_OBJ(l_ModelInstanceTextures, modelinstance_h, "modelinstancetextures", "Flat array of textures for one model instance.  It has the same layout as the model's \"textures\" handle: each material (texture_map) contributes " SCP_TOKEN_TO_STR(TM_NUM_TYPES) " consecutive entries, in this order: base, glow, specular, normal, height, misc, reflectance, ambient occlusion.  So for material N (1-based), the base map is at index (N-1)*" SCP_TOKEN_TO_STR(TM_NUM_TYPES) "+1, the glow map at (N-1)*" SCP_TOKEN_TO_STR(TM_NUM_TYPES) "+2, and so on.  Reads return the instance's replacement texture for that slot if one is set, otherwise the model's texture.  Writes set a replacement texture on this instance only.  The TextureMaps array exposes the same slots grouped per material, as texture_map handles.");

ADE_FUNC(__len, l_ModelInstanceTextures, nullptr, "Number of texture slots on the model instance, i.e. the number of materials multiplied by " SCP_TOKEN_TO_STR(TM_NUM_TYPES), "number", "Number of texture slots, or 0 if handle is invalid")
{
	modelinstance_h *mih;
	if(!ade_get_args(L, "o", l_ModelInstanceTextures.GetPtr(&mih)))
		return ade_set_error(L, "i", 0);

	if(!mih->isValid())
		return ade_set_error(L, "i", 0);

	polymodel *pm = model_get(mih->Get()->model_num);

	if(pm == nullptr)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_textures*TM_NUM_TYPES);
}

ADE_INDEXER(l_ModelInstanceTextures, "number/string IndexOrTextureFilename", "Gets or sets a texture slot.  A number is a 1-based index into the flat array (see the \"modelinstancetextures\" handle description for the layout).  A string is matched first against the filenames of this instance's replacement textures, then against the filename of every slot on every material of the model.  Setting a slot sets a replacement texture on this instance only; set it to an invalid texture handle to clear the replacement.  The instance takes its own reference to the texture, so the script does not need to keep the handle alive.", "texture", "Texture, or invalid texture handle if the handle is invalid or the index/name does not match")
{
	modelinstance_h *mih;
	const char* s;
	texture_h* tdx = nullptr;
	if (!ade_get_args(L, "os|o", l_ModelInstanceTextures.GetPtr(&mih), &s, l_Texture.GetPtr(&tdx)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (!mih->isValid() || s == nullptr)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	polymodel_instance *pmi = mih->Get();
	polymodel *pm = model_get(pmi->model_num);
	int final_index = model_instance_find_texture_slot(pmi, pm, s);

	if (final_index < 0)
	{
		final_index = atoi(s) - 1;	//Lua->FS2

		if (final_index < 0 || final_index >= MAX_REPLACEMENT_TEXTURES)
			return ade_set_error(L, "o", l_Texture.Set(texture_h()));
	}

	if (ADE_SETTING_VAR) {
		if (pmi->texture_replace == nullptr) {
			pmi->texture_replace = std::make_shared<model_texture_replace>();
		}

		// an invalid texture handle clears the replacement
		if (tdx != nullptr)
			pmi->texture_replace->reference(final_index, tdx->handle);
	}

	if (pmi->texture_replace != nullptr && (*pmi->texture_replace)[final_index] >= 0)
		return ade_set_args(L, "o", l_Texture.Set(texture_h((*pmi->texture_replace)[final_index])));
	else
		return ade_set_args(L, "o", l_Texture.Set(texture_h(pm->maps[final_index / TM_NUM_TYPES].textures[final_index % TM_NUM_TYPES].GetTexture())));
}


ADE_OBJ(l_ModelInstance, modelinstance_h, "model_instance", "Model instance handle");

modelinstance_h::modelinstance_h(int pmi_id)
{
	_pmi_id = pmi_id;
}
modelinstance_h::modelinstance_h(polymodel_instance *pmi)
	: _pmi_id(pmi ? pmi->id : -1)
{}
modelinstance_h::modelinstance_h()
	: _pmi_id(-1)
{}
polymodel_instance *modelinstance_h::Get() const
{
	return isValid() ? model_get_instance(_pmi_id) : nullptr;
}
int modelinstance_h::GetID() const
{
	return isValid() ? _pmi_id : -1;
}
polymodel *modelinstance_h::GetModel() const
{
	return isValid() ? model_get(model_get_instance(_pmi_id)->model_num) : nullptr;
}
int modelinstance_h::GetModelID() const
{
	return isValid() ? model_get_instance(_pmi_id)->model_num : -1;
}
bool modelinstance_h::isValid() const
{
	return (_pmi_id >= 0) && (_pmi_id < num_model_instances()) && (model_get_instance(_pmi_id) != nullptr);
}


ADE_OBJ(l_SubmodelInstance, submodelinstance_h, "submodel_instance", "Submodel instance handle");

submodelinstance_h::submodelinstance_h(int pmi_id, int submodel_num)
	: _pmi_id(pmi_id), _submodel_num(submodel_num)
{}
submodelinstance_h::submodelinstance_h(polymodel_instance *pmi, int submodel_num)
	: _pmi_id(pmi ? pmi->id : -1), _submodel_num(submodel_num)
{}
submodelinstance_h::submodelinstance_h()
	: _pmi_id(-1), _submodel_num(-1)
{}
polymodel_instance *submodelinstance_h::GetModelInstance() const
{
	return isValid() ? model_get_instance(_pmi_id) : nullptr;
}
int submodelinstance_h::GetModelInstanceID() const
{
	return isValid() ? _pmi_id : -1;
}
submodel_instance *submodelinstance_h::Get() const
{
	return isValid() ? &model_get_instance(_pmi_id)->submodel[_submodel_num] : nullptr;
}
polymodel *submodelinstance_h::GetModel() const
{
	return isValid() ? model_get(model_get_instance(_pmi_id)->model_num) : nullptr;
}
int submodelinstance_h::GetModelID() const
{
	return isValid() ? model_get_instance(_pmi_id)->model_num : -1;
}
bsp_info *submodelinstance_h::GetSubmodel() const
{
	return isValid() ? &model_get(model_get_instance(_pmi_id)->model_num)->submodel[_submodel_num] : nullptr;
}
int submodelinstance_h::GetSubmodelIndex() const
{
	return isValid() ? _submodel_num : -1;
}
bool submodelinstance_h::isValid() const
{
	if (_pmi_id >= 0 && _submodel_num >= 0 && _pmi_id < num_model_instances())
	{
		auto pmi = model_get_instance(_pmi_id);
		if (pmi != nullptr && pmi->model_num >= 0)
		{
			auto pm = model_get(pmi->model_num);
			if (pm != nullptr && _submodel_num < pm->n_models)
				return true;
		}
	}
	return false;
}


ADE_FUNC(__eq, l_ModelInstance, "model_instance, model_instance", "Checks if two model instance handles refer to the same model instance", "boolean", "True if model instances are equal")
{
	modelinstance_h* mih1;
	modelinstance_h* mih2;

	if (!ade_get_args(L, "oo", l_ModelInstance.GetPtr(&mih1), l_ModelInstance.GetPtr(&mih2)))
		return ADE_RETURN_NIL;

	if (mih1->GetID() == mih2->GetID())
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(__eq, l_SubmodelInstance, "submodel_instance, submodel_instance", "Checks if two submodel instance handles refer to the same submodel instance", "boolean", "True if submodel instances are equal")
{
	submodelinstance_h* smih1;
	submodelinstance_h* smih2;

	if (!ade_get_args(L, "oo", l_SubmodelInstance.GetPtr(&smih1), l_SubmodelInstance.GetPtr(&smih2)))
		return ADE_RETURN_NIL;

	if (smih1->GetModelInstanceID() == smih2->GetModelInstanceID() && smih1->GetSubmodelIndex() == smih2->GetSubmodelIndex())
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
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

ADE_VIRTVAR(Textures, l_ModelInstance, "modelinstancetextures", "Gets model instance textures", "modelinstancetextures", "Model instance textures, or invalid modelinstancetextures handle if modelinstance handle is invalid")
{
	modelinstance_h *sh = nullptr;
	modelinstance_h *dh;
	if(!ade_get_args(L, "o|o", l_ModelInstance.GetPtr(&dh), l_ModelInstance.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	if(!dh->isValid())
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	if(ADE_SETTING_VAR && sh && sh->isValid()) {
		dh->Get()->texture_replace = sh->Get()->texture_replace;
	}

	return ade_set_args(L, "o", l_ModelInstanceTextures.Set(modelinstance_h(dh->Get())));
}

ADE_VIRTVAR(TextureMaps, l_ModelInstance, "modelinstancetexturemaps", "Gets the model instance's materials.  Assigning another instance's TextureMaps makes this instance share that instance's replacement textures, exactly as assigning Textures does.", "modelinstancetexturemaps", "Array of the model instance's materials, or invalid modelinstancetexturemaps handle if modelinstance handle is invalid")
{
	modelinstance_h *sh = nullptr;
	modelinstance_h *dh;
	if(!ade_get_args(L, "o|o", l_ModelInstance.GetPtr(&dh), l_ModelInstance.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ModelInstanceTextureMaps.Set(modelinstance_h()));

	if(!dh->isValid())
		return ade_set_error(L, "o", l_ModelInstanceTextureMaps.Set(modelinstance_h()));

	if(ADE_SETTING_VAR && sh && sh->isValid())
		dh->Get()->texture_replace = sh->Get()->texture_replace;

	return ade_set_args(L, "o", l_ModelInstanceTextureMaps.Set(modelinstance_h(dh->Get())));
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

ADE_FUNC(getModelInstance, l_SubmodelInstance, nullptr, "Gets the model instance of this submodel", "model_instance", "A model instancve")
{
	submodelinstance_h* smih;
	if (!ade_get_args(L, "o", l_SubmodelInstance.GetPtr(&smih)))
		return ade_set_error(L, "o", l_ModelInstance.Set(modelinstance_h()));

	if (!smih->isValid())
		return ade_set_error(L, "o", l_ModelInstance.Set(modelinstance_h()));

	return ade_set_args(L, "o", l_ModelInstance.Set(modelinstance_h(smih->GetModelInstance())));
}

ADE_FUNC(getSubmodel, l_SubmodelInstance, nullptr, "Gets the submodel of this instance", "submodel", "A submodel")
{
	submodelinstance_h* smih;
	if (!ade_get_args(L, "o", l_SubmodelInstance.GetPtr(&smih)))
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (!smih->isValid())
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(smih->GetModel(), smih->GetSubmodelIndex())));
}

ADE_VIRTVAR(Orientation, l_SubmodelInstance, "orientation", "Gets or sets the submodel instance orientation", "orientation", "Orientation, or identity orientation if handle is not valid")
{
	submodelinstance_h *smih;
	matrix_h *mh = nullptr;
	if (!ade_get_args(L, "o|o", l_SubmodelInstance.GetPtr(&smih), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (!smih->isValid())
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

	if (!smih->isValid())
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

	if (!smih->isValid())
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

	if (!smih->isValid())
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

	if (!smih->isValid())
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

ADE_FUNC(findLocalPointAndOrientation, l_SubmodelInstance, "vector, orientation, [boolean world = true]", "Calculates the coordinates and orientation in the submodel's frame of reference, of a point and orientation in world coordinates [world = true] / in the object's frame of reference [world = false]", "vector, orientation", "Vector and orientation, or empty values if a handle is invalid")
{
	vec3d local_pnt;
	matrix local_orient;

	submodelinstance_h* smih;
	vec3d global_pnt;
	matrix_h* global_orient;
	bool fromWorld = true;

	if (!ade_get_args(L, "ooo|b", l_SubmodelInstance.GetPtr(&smih), l_Vector.Get(&global_pnt), l_Matrix.GetPtr(&global_orient), &fromWorld))
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Matrix.Set(matrix_h(&vmd_zero_matrix)));

	if (!smih->isValid())
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Matrix.Set(matrix_h(&vmd_zero_matrix)));

	auto pmi = smih->GetModelInstance();
	if (fromWorld && pmi->objnum < 0)
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Matrix.Set(matrix_h(&vmd_zero_matrix)));

	model_instance_global_to_local_point_orient(&local_pnt, &local_orient, &global_pnt, global_orient->GetMatrix(), smih->GetModel(), pmi, smih->GetSubmodelIndex(), fromWorld ? &Objects[pmi->objnum].orient : nullptr, fromWorld ? &Objects[pmi->objnum].pos : nullptr);
	
	return ade_set_args(L, "oo", l_Vector.Set(local_pnt), l_Matrix.Set(matrix_h(&local_orient)));
}


ADE_OBJ(l_ModelSubmodelInstances, modelsubmodelinstances_h, "submodel_instances", "Array of submodel instances");

modelsubmodelinstances_h::modelsubmodelinstances_h(polymodel_instance *pmi) : modelinstance_h(pmi){}
modelsubmodelinstances_h::modelsubmodelinstances_h() : modelinstance_h(){}

ADE_FUNC(__len, l_ModelSubmodelInstances, nullptr, "Number of submodel instances on model", "number", "Number of model submodel instances")
{
	modelsubmodelinstances_h *msih;
	if (!ade_get_args(L, "o", l_ModelSubmodelInstances.GetPtr(&msih)))
		return ade_set_error(L, "i", 0);

	if (!msih->isValid())
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

		if (!msih->isValid())
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index--; // Lua --> C/C++
	}
	else if (lua_isstring(L, 2))
	{
		const char *name = nullptr;

		if (!ade_get_args(L, "os", l_ModelSubmodelInstances.GetPtr(&msih), &name))
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		if (!msih->isValid() || name == nullptr)
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index = model_find_submodel_index(msih->Get()->model_num, name);
	}
	else
	{
		submodel_h *smh;

		if (!ade_get_args(L, "oo", l_ModelSubmodelInstances.GetPtr(&msih), l_Submodel.GetPtr(&smh)))
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		if (!msih->isValid() || !smh->isValid())
			return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

		index = smh->GetSubmodelIndex();
	}

	auto pmi = msih->Get();
	auto pm = model_get(pmi->model_num);

	if (index < 0 || index >= pm->n_models)
		return ade_set_error(L, "o", l_SubmodelInstance.Set(submodelinstance_h()));

	return ade_set_args(L, "o", l_SubmodelInstance.Set(submodelinstance_h(pmi, index)));
}

}
}

//
//

#include "model.h"
#include "vecmath.h"
#include "eye.h"
#include "texture.h"

extern void model_calc_bound_box(vec3d *box, const vec3d *big_mn, const vec3d *big_mx);

namespace scripting {
namespace api {

ADE_OBJ(l_Model, model_h, "model", "3D Model (POF) handle");

polymodel *model_h::Get() const
{
	return isValid() ? model_get(model_num) : nullptr;
}
int model_h::GetID() const
{
	return isValid() ? model_num : -1;
}
bool model_h::isValid() const
{
	return (model_num >= 0) && (model_get(model_num) != nullptr);	// note: the model ID can exceed MAX_POLYGON_MODELS because the modulo is taken
}
model_h::model_h(int n_modelnum)
	: model_num(n_modelnum)
{}
model_h::model_h(polymodel *n_model)
	: model_h(n_model->id)
{}
model_h::model_h()
	: model_h(-1)
{}


ADE_OBJ(l_Submodel, submodel_h, "submodel", "Handle to a submodel");

submodel_h::submodel_h(int n_modelnum, int n_submodelnum)
	: model_num(n_modelnum), submodel_num(n_submodelnum)
{}
submodel_h::submodel_h(polymodel *n_model, int n_submodelnum)
	: submodel_h(n_model->id, n_submodelnum)
{}
submodel_h::submodel_h()
	: submodel_h(-1, -1)
{}
polymodel *submodel_h::GetModel() const { return isValid() ? model_get(model_num) : nullptr; }
int submodel_h::GetModelID() const { return isValid() ? model_num : -1; }
bsp_info* submodel_h::GetSubmodel() const { return isValid() ? &model_get(model_num)->submodel[submodel_num] : nullptr; }
int submodel_h::GetSubmodelIndex() const { return isValid() ? submodel_num : -1; }
bool submodel_h::isValid() const
{
	if (model_num >= 0 && submodel_num >= 0)
	{
		auto model = model_get(model_num);
		if (model != nullptr && submodel_num < model->n_models)
			return true;
	}
	return false;
}


ADE_FUNC(__eq, l_Model, "model, model", "Checks if two model handles refer to the same model", "boolean", "True if models are equal")
{
	model_h* mdl1;
	model_h* mdl2;

	if (!ade_get_args(L, "oo", l_Model.GetPtr(&mdl1), l_Model.GetPtr(&mdl2)))
		return ADE_RETURN_NIL;

	if (mdl1->GetID() == mdl2->GetID())
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(__eq, l_Submodel, "submodel, submodel", "Checks if two submodel handles refer to the same submodel", "boolean", "True if submodels are equal")
{
	submodel_h* smh1;
	submodel_h* smh2;

	if (!ade_get_args(L, "oo", l_Submodel.GetPtr(&smh1), l_Submodel.GetPtr(&smh2)))
		return ADE_RETURN_NIL;

	if (smh1->GetModelID() == smh2->GetModelID() && smh1->GetSubmodelIndex() == smh2->GetSubmodelIndex())
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Submodels, l_Model, nullptr, "Model submodels", "submodels", "Model submodels, or an invalid submodels handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelSubmodels.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelSubmodels.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning submodels is not supported");

	return ade_set_args(L, "o", l_ModelSubmodels.Set(model_h(pm)));
}

ADE_VIRTVAR(Textures, l_Model, nullptr, "Model textures", "textures", "Model textures, or an invalid textures handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelTextures.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelTextures.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning textures is not supported");

	return ade_set_args(L, "o", l_ModelTextures.Set(model_h(pm)));
}

ADE_VIRTVAR(Thrusters, l_Model, nullptr, "Model thrusters", "thrusters", "Model thrusters, or an invalid thrusters handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelThrusters.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelThrusters.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning thrusters is not supported");

	return ade_set_args(L, "o", l_ModelThrusters.Set(model_h(pm)));
}

ADE_VIRTVAR(GlowPointBanks, l_Model, nullptr, "Model glow point banks", "glowpointbanks", "Model glow point banks, or an invalid glowpointbanks handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelGlowpointbanks.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelGlowpointbanks.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning glow point banks is not supported");

	return ade_set_args(L, "o", l_ModelGlowpointbanks.Set(model_h(pm)));
}

ADE_VIRTVAR(Eyepoints, l_Model, nullptr, "Model eyepoints", "eyepoints", "Array of eyepoints, or an invalid eyepoints handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelEyepoints.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelEyepoints.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning eye points is not supported");

	return ade_set_args(L, "o", l_ModelEyepoints.Set(model_h(pm)));
}

ADE_VIRTVAR(Dockingbays, l_Model, nullptr, "Model docking bays", "dockingbays", "Array of docking bays, or an invalid dockingbays handle if the model handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_ModelDockingbays.Set(model_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_ModelDockingbays.Set(model_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning docking bays is not supported");

	return ade_set_args(L, "o", l_ModelDockingbays.Set(model_h(pm)));
}

ADE_VIRTVAR(BoundingBoxMax, l_Model, "vector", "Model bounding box maximum", "vector", "Model bounding box, or an empty vector if the handle is not valid")
{
	model_h *mdl = NULL;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL) {
		pm->maxs = *v;

		//Recalculate this, so it stays valid
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->maxs));
}

ADE_VIRTVAR(BoundingBoxMin, l_Model, "vector", "Model bounding box minimum", "vector", "Model bounding box, or an empty vector if the handle is not valid")
{
	model_h *mdl = NULL;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL) {
		pm->mins = *v;

		//Recalculate this, so it stays valid
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->mins));
}

ADE_VIRTVAR(Filename, l_Model, "string", "Model filename", "string", "Model filename, or an empty string if the handle is not valid")
{
	model_h *mdl = NULL;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Model.GetPtr(&mdl), &s))
		return ade_set_error(L, "s", "");

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		auto len = sizeof(pm->filename);
		strncpy(pm->filename, s, len);
		pm->filename[len - 1] = 0;
	}

	return ade_set_args(L, "s", pm->filename);
}

ADE_VIRTVAR(Mass, l_Model, "number", "Model mass", "number", "Model mass, or 0 if the model handle is invalid")
{
	model_h *mdl = NULL;
	float nm = 0.0f;
	if(!ade_get_args(L, "o|f", l_Model.GetPtr(&mdl), &nm))
		return ade_set_error(L, "f", 0.0f);

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pm->mass = nm;
	}

	return ade_set_args(L, "f", pm->mass);
}

ADE_VIRTVAR(MomentOfInertia, l_Model, "orientation", "Model moment of inertia", "orientation", "Moment of Inertia matrix or identity matrix if invalid" )
{
	model_h *mdl = NULL;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "o|o", l_Model.GetPtr(&mdl), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh != NULL) {
		matrix *mtx = mh->GetMatrix();
		memcpy(&pm->moment_of_inertia, mtx, sizeof(*mtx));
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&pm->moment_of_inertia)));
}

ADE_VIRTVAR(Radius, l_Model, "number", "Model radius (Used for collision & culling detection)", "number", "Model Radius or 0 if invalid")
{
	model_h *mdl = NULL;
	float nr = 0.0f;
	if(!ade_get_args(L, "o|f", l_Model.GetPtr(&mdl), &nr))
		return ade_set_error(L, "f", 0.0f);

	polymodel *pm = mdl->Get();

	if(pm == NULL)
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pm->rad = nr;
	}

	return ade_set_args(L, "f", pm->rad);
}

ADE_FUNC(getDetailRoot, l_Model, "[number detailLevel]", "Returns the root submodel of the specified detail level - 0 for detail0, etc.", "submodel", "A submodel, or an invalid submodel if handle is not valid")
{
	model_h *mdl;
	int detail = 0;
	if (!ade_get_args(L, "o|i", l_Model.GetPtr(&mdl), &detail))
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	auto pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (detail < 0 || detail >= pm->n_detail_levels)
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(pm, pm->detail[detail])));
}

ADE_FUNC(isValid, l_Model, nullptr, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	model_h *mdl;
	if (!ade_get_args(L, "o", l_Model.GetPtr(&mdl)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", mdl->isValid());
}

ADE_VIRTVAR(Name, l_Submodel, nullptr, "Gets the submodel's name", "string", "The name or an empty string if invalid")
{
	submodel_h *smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "s", "");

	if (!smh->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the submodel name is not implemented");

	return ade_set_args(L, "s", smh->GetSubmodel()->name);
}

ADE_VIRTVAR(Index, l_Submodel, nullptr, "Gets the submodel's index", "number", "The number (adjusted for lua) or -1 if invalid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "i", -1);

	if (!smh->isValid())
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the submodel index is not implemented");

	return ade_set_args(L, "i", smh->GetSubmodelIndex() + 1);
}

ADE_VIRTVAR(Offset, l_Submodel, nullptr, "Gets the submodel's offset from its parent submodel", "vector", "The offset vector or a empty vector if invalid")
{
	submodel_h *smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!smh->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the submodel offset is not implemented");

	return ade_set_args(L, "o", l_Vector.Set(smh->GetSubmodel()->offset));
}

ADE_VIRTVAR(Radius, l_Submodel, nullptr, "Gets the submodel's radius", "number", "The radius of the submodel or -1 if invalid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "f", -1.0f);

	if (!smh->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the submodel radius is not implemented");

	return ade_set_args(L, "f", smh->GetSubmodel()->rad);
}

ADE_FUNC(NumVertices, l_Submodel, nullptr, "Returns the number of vertices in the submodel's mesh", "number", "The number of vertices, or 0 if the submodel was invalid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "i", 0);

	if (!smh->isValid())
		return ade_set_error(L, "i", 0);

	auto sm = smh->GetSubmodel();
	bsp_collision_tree* tree = model_get_bsp_collision_tree(sm->collision_tree_index);

	return ade_set_args(L, "i", tree->n_verts);
}

ADE_FUNC(GetVertex, l_Submodel, "[number index]", "Gets the specified vertex, or a random one if no index specified", "vector", "The vertex position in the submodel's frame of reference, or nil if the submodel was invalid")
{
	submodel_h* smh = nullptr;
	int idx = -1;

	if (!ade_get_args(L, "o|i", l_Submodel.GetPtr(&smh), &idx))
		return ADE_RETURN_NIL;

	if (!smh->isValid())
		return ADE_RETURN_NIL;

	auto sm = smh->GetSubmodel(); 
	bsp_collision_tree* tree = model_get_bsp_collision_tree(sm->collision_tree_index);

	if (idx >= tree->n_verts)
		return ADE_RETURN_NIL;

	vec3d vert;

	if (idx < 0) {
		vert = submodel_get_random_point(smh->GetModelID(), smh->GetSubmodelIndex());
	} else {
		vert = tree->point_list[idx];
	}

	return ade_set_args(L, "o", l_Vector.Set(vert));
}

ADE_FUNC(getModel, l_Submodel, nullptr, "Gets the model that this submodel belongs to", "model", "A model, or an invalid model if the handle is not valid")
{
	submodel_h *smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	if (!smh->isValid())
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	return ade_set_args(L, "o", l_Model.Set(model_h(smh->GetModelID())));
}

ADE_FUNC(getFirstChild, l_Submodel, nullptr, "Gets the first child submodel of this submodel", "submodel", "A submodel, or nil if there is no child, or an invalid submodel if the handle is not valid")
{
	submodel_h *smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (!smh->isValid())
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	auto sm = smh->GetSubmodel();
	if (sm->first_child < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(smh->GetModel(), sm->first_child)));
}

ADE_FUNC(getNextSibling, l_Submodel, nullptr, "Gets the next sibling submodel of this submodel", "submodel", "A submodel, or nil if there are no remaining siblings, or an invalid submodel if the handle is not valid")
{
	submodel_h *smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (!smh->isValid())
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	auto sm = smh->GetSubmodel();
	if (sm->next_sibling < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(smh->GetModel(), sm->next_sibling)));
}

ADE_FUNC(getParent, l_Submodel, nullptr, "Gets the parent submodel of this submodel", "submodel", "A submodel, or nil if there is no parent, or an invalid submodel if the handle is not valid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (!smh->isValid())
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	auto sm = smh->GetSubmodel();
	if (sm->parent < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(smh->GetModel(), sm->parent)));
}

ADE_FUNC(isValid, l_Submodel, nullptr, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	submodel_h *smh;
	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", smh->isValid());
}

ADE_VIRTVAR(NoCollide, l_Submodel, nullptr, "Whether the submodel and its children ignore collisions", "boolean", "The flag, or error-false if invalid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "b", false);

	if (!smh->isValid())
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting NoCollide is not supported");

	return ade_set_args(L, "b", smh->GetSubmodel()->flags[Model::Submodel_flags::No_collisions]);
}

ADE_VIRTVAR(NoCollideThisOnly, l_Submodel, nullptr, "Whether the submodel itself ignores collisions", "boolean", "The flag, or error-false if invalid")
{
	submodel_h* smh = nullptr;

	if (!ade_get_args(L, "o", l_Submodel.GetPtr(&smh)))
		return ade_set_error(L, "b", false);

	if (!smh->isValid())
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting NoCollideThisOnly is not supported");

	return ade_set_args(L, "b", smh->GetSubmodel()->flags[Model::Submodel_flags::Nocollide_this_only]);
}


//**********HANDLE: modelsubmodels
ADE_OBJ(l_ModelSubmodels, model_h, "submodels", "Array of submodels");

ADE_FUNC(__len, l_ModelSubmodels, nullptr, "Number of submodels on model", "number", "Number of model submodels")
{
	model_h *msh;
	if (!ade_get_args(L, "o", l_ModelSubmodels.GetPtr(&msh)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = msh->Get();
	if (!pm)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_models);
}

ADE_INDEXER(l_ModelSubmodels, "submodel", "number|string IndexOrName", "submodel", "Model submodels, or invalid modelsubmodels handle if model handle is invalid")
{
	model_h *msh = nullptr;
	int index = -1;
	polymodel *pm = nullptr;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi", l_ModelSubmodels.GetPtr(&msh), &index))
			return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

		pm = msh->Get();
		if (!pm)
			return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

		index--; // Lua --> C/C++
	}
	else
	{
		const char *name = nullptr;

		if (!ade_get_args(L, "os", l_ModelSubmodels.GetPtr(&msh), &name))
			return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

		pm = msh->Get();
		if (!pm)
			return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

		index = model_find_submodel_index(pm->id, name);
	}

	if (index < 0 || index >= pm->n_models)
		return ade_set_error(L, "o", l_Submodel.Set(submodel_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting submodels is not supported");

	return ade_set_args(L, "o", l_Submodel.Set(submodel_h(pm, index)));
}

ADE_FUNC(isValid, l_ModelSubmodels, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if invalid, nil if a syntax/type error occurs")
{
	model_h *msh;
	if (!ade_get_args(L, "o", l_ModelSubmodels.GetPtr(&msh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", msh->isValid());
}


//**********HANDLE: modeltextures
ADE_OBJ(l_ModelTextures, model_h, "textures", "Array of textures");

ADE_FUNC(__len, l_ModelTextures, NULL, "Number of textures on model", "number", "Number of model textures")
{
	model_h *mth;
	if (!ade_get_args(L, "o", l_ModelTextures.GetPtr(&mth)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = mth->Get();
	if (!pm)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", TM_NUM_TYPES * pm->n_textures);
}

ADE_INDEXER(l_ModelTextures, "texture", "number Index/string TextureName", "texture", "Model textures, or invalid modeltextures handle if model handle is invalid")
{
	model_h *mth = NULL;
	texture_h* new_tex   = nullptr;
	const char* s        = nullptr;

	if (!ade_get_args(L, "os|o", l_ModelTextures.GetPtr(&mth), &s, l_Texture.GetPtr(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	polymodel *pm = mth->Get();
	if (s == nullptr || pm == nullptr)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	texture_info *tinfo = NULL;
	texture_map *tmap = NULL;

	if(strspn(s, "0123456789") == strlen(s))
	{
		int num_textures = TM_NUM_TYPES*pm->n_textures;
		int idx = atoi(s) - 1;	//Lua->FS2

		if (idx < 0 || idx >= num_textures)
			return ade_set_error(L, "o", l_Texture.Set(texture_h()));

		tmap = &pm->maps[idx / TM_NUM_TYPES];
		tinfo = &tmap->textures[idx % TM_NUM_TYPES];
	}

	if(tinfo == NULL)
	{
		for (int i = 0; i < pm->n_textures; i++)
		{
			tmap = &pm->maps[i];

			int tnum = tmap->FindTexture(s);
			if(tnum > -1)
				tinfo = &tmap->textures[tnum];
		}
	}

	if(tinfo == NULL)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (ADE_SETTING_VAR && new_tex != nullptr) {
		tinfo->SetTexture(new_tex->handle);
	}

	return ade_set_args(L, "o", l_Texture.Set(texture_h(tinfo->GetTexture())));
}

ADE_FUNC(isValid, l_ModelTextures, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	model_h *mth;
	if(!ade_get_args(L, "o", l_ModelTextures.GetPtr(&mth)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", mth->isValid());
}


//**********HANDLE: eyepoints
ADE_OBJ(l_ModelEyepoints, model_h, "eyepoints", "Array of model eye points");

ADE_FUNC(__len, l_ModelEyepoints, NULL, "Gets the number of eyepoints on this model", "number", "Number of eyepoints on this model or 0 on error")
{
	model_h *eph = nullptr;
	if (!ade_get_args(L, "o", l_ModelEyepoints.GetPtr(&eph)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = eph->Get();
	if (!pm)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_view_positions);
}

ADE_INDEXER(l_ModelEyepoints, "eyepoint", "Gets an eyepoint handle", "eyepoint", "eye handle or invalid handle on error")
{
	model_h *mdl = nullptr;
	int index = -1;

	if (!ade_get_args(L, "oi", l_ModelEyepoints.GetPtr(&mdl), &index))
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));

	index--; // Lua -> FS2
	if (index < 0 || index >= pm->n_view_positions)
		return ade_set_error(L, "o", l_Eyepoint.Set(eye_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning eye points is not supported");

	return ade_set_args(L, "o", l_Eyepoint.Set(eye_h(pm->id, index)));
}

ADE_FUNC(isValid, l_ModelEyepoints, NULL, "Detects whether handle is valid or not", "boolean", "true if valid false otherwise")
{
	model_h *eph;
	if(!ade_get_args(L, "o", l_ModelEyepoints.GetPtr(&eph)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", eph->isValid());
}

//**********HANDLE: thrusters
ADE_OBJ(l_ModelThrusters, model_h, "thrusters", "The thrusters of a model");

ADE_FUNC(__len, l_ModelThrusters, NULL, "Number of thruster banks on the model", "number", "Number of thrusterbanks")
{
	model_h *mdl;
	if (!ade_get_args(L, "o", l_ModelThrusters.GetPtr(&mdl)))
		return ade_set_error(L, "i", -1);

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", pm->n_thrusters);
}

ADE_INDEXER(l_ModelThrusters, "number Index", "Array of all thrusterbanks on this thruster", "thrusterbank", "Handle to the thrusterbank or invalid handle if index is invalid")
{
	model_h *mdl = nullptr;
	int idx = -1;

	if (!ade_get_args(L, "oi", l_ModelThrusters.GetPtr(&mdl), &idx))
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	polymodel *pm = mdl->Get();
	if (!pm)
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	idx--;	//Lua->FS2
	if (idx < 0 || idx >= pm->n_thrusters)
		return ade_set_error(L, "o", l_Thrusterbank.Set(thrusterbank_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning thruster banks is not supported");

	return ade_set_args(L, "o", l_Thrusterbank.Set(thrusterbank_h(pm->id, idx)));
}

ADE_FUNC(isValid, l_ModelThrusters, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	model_h *trh;
	if(!ade_get_args(L, "o", l_ModelThrusters.GetPtr(&trh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", trh->isValid());
}

//**********HANDLE: thrusterbank
ADE_OBJ(l_Thrusterbank, thrusterbank_h, "thrusterbank", "A model thrusterbank");

thrusterbank_h::thrusterbank_h(int in_model_num, int in_thrusterbank_index)
	: modelh(in_model_num), thrusterbank_index(in_thrusterbank_index)
{}
thrusterbank_h::thrusterbank_h()
	: modelh(), thrusterbank_index(-1)
{}
thruster_bank* thrusterbank_h::Get() const
{
	if (!isValid())
		return nullptr;

	// coverity[returned_null:FALSE] - isValid() specifically checks for modelh.Get() returning null
	return &modelh.Get()->thrusters[thrusterbank_index];
}
bool thrusterbank_h::isValid() const
{
	if (thrusterbank_index < 0)
		return false;
	auto model = modelh.Get();
	if (!model)
		return false;
	return thrusterbank_index < model->n_thrusters;
}

ADE_FUNC(__len, l_Thrusterbank, NULL, "Number of thrusters on this thrusterbank", "number", "Number of thrusters on this bank or 0 if handle is invalid")
{
	thrusterbank_h *tbh = NULL;
	if(!ade_get_args(L, "o", l_Thrusterbank.GetPtr(&tbh)))
		return ade_set_error(L, "i", -1);

	if(!tbh->isValid())
		return ade_set_error(L, "i", -1);

	thruster_bank* bank = tbh->Get();

	return ade_set_args(L, "i", bank->num_points);
}

ADE_INDEXER(l_Thrusterbank, "number Index", "Array of glowpoint", "glowpoint", "Glowpoint, or invalid glowpoint handle on failure")
{
	thrusterbank_h *tbh = nullptr;
	int idx = -1;

	if (!ade_get_args(L, "oi", l_Thrusterbank.GetPtr(&tbh), &idx))
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	thruster_bank* bank = tbh->Get();
	if (!bank)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	idx--; // Lua -> FS2
	if (idx < 0 || idx >= bank->num_points)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning glow points is not supported");

	return ade_set_args(L, "o", l_Glowpoint.Set(glowpoint_h(tbh->modelh.GetID(), -1, tbh->thrusterbank_index, idx)));
}

ADE_FUNC(isValid, l_Thrusterbank, nullptr, "Detects if this handle is valid", "boolean", "true if this handle is valid, false otherwise")
{
	thrusterbank_h* trh;
	if(!ade_get_args(L, "o", l_Thrusterbank.GetPtr(&trh)))
		return ADE_RETURN_FALSE;

	if (!trh->isValid())
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", trh->isValid());
}

//**********HANDLE: glow point banks
ADE_OBJ(l_ModelGlowpointbanks, model_h, "glowpointbanks", "Array of model glow point banks");

ADE_FUNC(__len, l_ModelGlowpointbanks, nullptr, "Gets the number of glow point banks on this model", "number", "Number of glow point banks on this model or 0 on error")
{
	model_h *modelh = nullptr;
	if (!ade_get_args(L, "o", l_ModelGlowpointbanks.GetPtr(&modelh)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = modelh->Get();
	if (!pm)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_glow_point_banks);
}

ADE_INDEXER(l_ModelGlowpointbanks, "glowpointbank", "Gets a glow point bank handle", "glowpointbank", "glowpointbank handle or invalid handle on error")
{
	model_h *modelh = nullptr;
	int index = -1;

	if (!ade_get_args(L, "oi", l_ModelGlowpointbanks.GetPtr(&modelh), &index))
		return ade_set_error(L, "o", l_Glowpointbank.Set(glowpointbank_h()));

	polymodel *pm = modelh->Get();
	if (!pm)
		return ade_set_error(L, "o", l_Glowpointbank.Set(glowpointbank_h()));

	index--; // Lua -> FS2
	if (index < 0 || index >= pm->n_glow_point_banks)
		return ade_set_error(L, "o", l_Glowpointbank.Set(glowpointbank_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning glow point banks is not supported");

	return ade_set_args(L, "o", l_Glowpointbank.Set(glowpointbank_h(pm->id, index)));
}

ADE_FUNC(isValid, l_ModelGlowpointbanks, nullptr, "Detects whether handle is valid or not", "boolean", "true if valid false otherwise")
{
	model_h *modelh;
	if(!ade_get_args(L, "o", l_ModelGlowpointbanks.GetPtr(&modelh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", modelh->isValid());
}

//**********HANDLE: glow point bank
ADE_OBJ(l_Glowpointbank, glowpointbank_h, "glowpointbank", "A model glow point bank");

glowpointbank_h::glowpointbank_h(int in_model_num, int in_glowpointbank_index)
	: modelh(in_model_num), glowpointbank_index(in_glowpointbank_index)
{}
glowpointbank_h::glowpointbank_h()
	: modelh(), glowpointbank_index(-1)
{}
glow_point_bank* glowpointbank_h::Get() const
{
	if (!isValid())
		return nullptr;

	// coverity[returned_null:FALSE] - isValid() specifically checks for modelh.Get() returning null
	return &modelh.Get()->glow_point_banks[glowpointbank_index];
}
bool glowpointbank_h::isValid() const
{
	if (glowpointbank_index < 0)
		return false;
	auto model = modelh.Get();
	if (!model)
		return false;
	return glowpointbank_index < model->n_glow_point_banks;
}

ADE_FUNC(__len, l_Glowpointbank, nullptr, "Gets the number of glow points in this bank", "number", "Number of glow points in this bank or 0 on error")
{
	glowpointbank_h *gpbh = nullptr;
	if (!ade_get_args(L, "o", l_Glowpointbank.GetPtr(&gpbh)))
		return ade_set_error(L, "i", 0);

	auto gpb = gpbh->Get();
	if (!gpb)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gpb->num_points);
}

ADE_INDEXER(l_Glowpointbank, "glowpoint", "Gets a glow point handle", "glowpoint", "glowpoint handle or invalid handle on error")
{
	glowpointbank_h *gpbh = nullptr;
	int index = -1;

	if (!ade_get_args(L, "oi", l_Glowpointbank.GetPtr(&gpbh), &index))
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	auto gpb = gpbh->Get();
	if (!gpb)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	index--; // Lua -> FS2
	if (index < 0 || index >= gpb->num_points)
		return ade_set_error(L, "o", l_Glowpoint.Set(glowpoint_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning glow points is not supported");

	return ade_set_args(L, "o", l_Glowpoint.Set(glowpoint_h(gpbh->modelh.GetID(), gpbh->glowpointbank_index, -1, index)));
}

// **********
// NOTE: Any fields or functions of glowpointbank will need to take glowpoint_bank_overrides into account
// **********

ADE_FUNC(isValid, l_Glowpointbank, nullptr, "Detects whether handle is valid or not", "boolean", "true if valid false otherwise")
{
	glowpointbank_h* gpbh = nullptr;
	if (!ade_get_args(L, "o", l_Glowpointbank.GetPtr(&gpbh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", gpbh->isValid());
}

//**********HANDLE: glowpoint
ADE_OBJ(l_Glowpoint, glowpoint_h, "glowpoint", "A model glowpoint");

glowpoint_h::glowpoint_h(int in_model_num, int in_glowpointbank_index, int in_thrusterbank_index, int in_glowpoint_index)
	: glowpointbankh(in_model_num, in_glowpointbank_index), thrusterbankh(in_model_num, in_thrusterbank_index), glowpoint_index(in_glowpoint_index)
{}
glowpoint_h::glowpoint_h()
	: glowpoint_h(-1, -1, -1, -1)
{}
glow_point* glowpoint_h::Get() const
{
	if (glowpoint_index < 0)
		return nullptr;

	auto gbank = glowpointbankh.Get();
	if (gbank)
	{
		if (glowpoint_index < gbank->num_points)
			return &gbank->points[glowpoint_index];
		else
			return nullptr;
	}
	auto tbank = thrusterbankh.Get();
	if (tbank)
	{
		if (glowpoint_index < tbank->num_points)
			return &tbank->points[glowpoint_index];
		else
			return nullptr;
	}

	return nullptr;
}
bool glowpoint_h::isValid() const
{
	if (glowpoint_index < 0)
		return false;

	auto gbank = glowpointbankh.Get();
	if (gbank)
		return (glowpoint_index < gbank->num_points);

	auto tbank = thrusterbankh.Get();
	if (tbank)
		return (glowpoint_index < tbank->num_points);

	return false;
}

ADE_VIRTVAR(Position, l_Glowpoint, nullptr, "The (local) vector to the position of the glowpoint", "vector", "The local vector to the glowpoint or nil if invalid")
{
	glowpoint_h *glh = nullptr;

	if (!ade_get_args(L, "o", l_Glowpoint.GetPtr(&glh)))
		return ADE_RETURN_NIL;

	auto point = glh->Get();
	if (!point)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "o", l_Vector.Set(point->pnt));
}

ADE_VIRTVAR(Normal, l_Glowpoint, nullptr, "The normal of the glowpoint", "vector", "The normal of the glowpoint or nil if invalid")
{
	glowpoint_h *glh = nullptr;

	if (!ade_get_args(L, "o", l_Glowpoint.GetPtr(&glh)))
		return ADE_RETURN_NIL;

	auto point = glh->Get();
	if (!point)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "o", l_Vector.Set(point->norm));
}

ADE_VIRTVAR(Radius, l_Glowpoint, nullptr, "The radius of the glowpoint", "number", "The radius of the glowpoint or -1 if invalid")
{
	glowpoint_h* glh = nullptr;

	if (!ade_get_args(L, "o", l_Glowpoint.GetPtr(&glh)))
		return ade_set_error(L, "f", -1.0f);

	auto point = glh->Get();
	if (!point)
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "f", point->radius);
}

ADE_FUNC(isValid, l_Glowpoint, NULL, "Returns whether this handle is valid or not", "boolean", "True if handle is valid, false otherwise")
{
	glowpoint_h glh;

	if(!ade_get_args(L, "o", l_Glowpoint.Get(&glh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", glh.isValid());
}

//**********HANDLE: dockingbays
ADE_OBJ(l_ModelDockingbays, model_h, "dockingbays", "The docking bays of a model");

ADE_INDEXER(l_ModelDockingbays, "dockingbay", "Gets a dockingbay handle from this model. If a string is given then a dockingbay with that name is searched.", "dockingbay", "Handle or invalid handle on error")
{
	model_h *dbhp = nullptr;
	int index = -1;
	polymodel *pm = nullptr;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi", l_ModelDockingbays.GetPtr(&dbhp), &index))
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		pm = dbhp->Get();
		if (!pm)
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		index--; // Lua --> C/C++
	}
	else
	{
		const char* name = nullptr;

		if (!ade_get_args(L, "os", l_ModelDockingbays.GetPtr(&dbhp), &name))
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		pm = dbhp->Get();
		if (!pm || !name)
			return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

		index = model_find_dock_name_index(pm->id, name);
	}

	if (index < 0 || index >= pm->n_docks)
		return ade_set_error(L, "o", l_Dockingbay.Set(dockingbay_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning docking bays is not supported");

	return ade_set_args(L, "o", l_Dockingbay.Set(dockingbay_h(pm, index)));
}

ADE_FUNC(__len, l_ModelDockingbays, NULL, "Retrieves the number of dockingbays on this model", "number", "number of docking bays or 0 on error")
{
	model_h *dbhp = NULL;

	if (!ade_get_args(L, "o", l_ModelDockingbays.GetPtr(&dbhp)))
		return ade_set_error(L, "i", 0);

	if (!dbhp->isValid())
		return ade_set_error(L, "i", 0);

	// coverity[returned_null:FALSE] - isValid() specifically checks for model_get() returning null
	return ade_set_args(L, "i", dbhp->Get()->n_docks);
}

//**********HANDLE: dockingbay
ADE_OBJ(l_Dockingbay, dockingbay_h, "dockingbay", "Handle to a model docking bay");

dockingbay_h::dockingbay_h(polymodel* pm, int dock_idx) : modelh(pm), dock_id(dock_idx) {}
dockingbay_h::dockingbay_h() : modelh(), dock_id(-1){}
bool dockingbay_h::isValid() const
{
	if (!modelh.isValid())
		return false;

	// coverity[returned_null:FALSE] - isValid() specifically checks for model_get() returning null
	return dock_id >= 0 && dock_id < modelh.Get()->n_docks;
}
dock_bay* dockingbay_h::getDockingBay() const
{
	if (!isValid())
		return nullptr;

	// coverity[returned_null:FALSE] - isValid() specifically checks for model_get() returning null
	return &modelh.Get()->docking_bays[dock_id];
}

ADE_FUNC(__len, l_Dockingbay, NULL, "Gets the number of docking points in this bay", "number", "The number of docking points or 0 on error")
{
	dockingbay_h* dbh = NULL;

	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ade_set_error(L, "i", 0);
	}

	if (dbh == NULL || !dbh->isValid())
	{
		return ade_set_error(L, "i", 0);
	}

	return ade_set_args(L, "i", dbh->getDockingBay()->num_slots);
}

ADE_FUNC(getName, l_Dockingbay, NULL, "Gets the name of this docking bay", "string", "The name or an empty string on error")
{
	dockingbay_h* dbh = NULL;
	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ade_set_error(L, "s", "");
	}

	if (dbh == NULL || !dbh->isValid())
	{
		return ade_set_error(L, "s", "");
	}

	dock_bay* dbp = dbh->getDockingBay();

	return ade_set_args(L, "s", dbp->name);
}

ADE_FUNC(getPoint, l_Dockingbay, "number index", "Gets the location of a docking point in this bay", "vector", "The local location or empty vector on error")
{
	dockingbay_h* dbh = NULL;
	int index = -1;

	if (!ade_get_args(L, "oi", l_Dockingbay.GetPtr(&dbh), &index))
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	index--; // Lua --> C/C++

	if (dbh == NULL || !dbh->isValid())
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	dock_bay* dbp = dbh->getDockingBay();

	if (index < 0 || index > dbp->num_slots)
	{
		LuaError(L, "Invalid dock bay index %d!", (index + 1));
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(dbp->pnt[index]));
}

ADE_FUNC(getNormal, l_Dockingbay, "number index", "Gets the normal of a docking point in this bay", "vector", "The normal vector or empty vector on error")
{
	dockingbay_h* dbh = NULL;
	int index = -1;

	if (!ade_get_args(L, "oi", l_Dockingbay.GetPtr(&dbh), &index))
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	index--; // Lua --> C/C++

	if (dbh == NULL || !dbh->isValid())
	{
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	dock_bay* dbp = dbh->getDockingBay();

	if (index < 0 || index > dbp->num_slots)
	{
		LuaError(L, "Invalid dock bay index %d!", (index + 1));
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(dbp->norm[index]));
}

ADE_FUNC(computeDocker, l_Dockingbay, "dockingbay",
         "Computes the final position and orientation of a docker bay that docks with this bay.",
         "vector, orientation",
         "The local location and orientation of the docker vessel in the reference to the vessel of the docking bay "
         "handle, or a nil value on error")
{
	dockingbay_h *dockee_bay_h = NULL, *docker_bay_h = NULL;
	vec3d final_pos;
	matrix final_orient;

	if (!ade_get_args(L, "oo", l_Dockingbay.GetPtr(&dockee_bay_h), l_Dockingbay.GetPtr(&docker_bay_h)))
	{
		return ADE_RETURN_NIL;
	}

	if (!dockee_bay_h->isValid() || !docker_bay_h->isValid())
	{
		return ADE_RETURN_NIL;
	}

	dock_bay* dockee_bay = dockee_bay_h->getDockingBay();
	dock_bay* docker_bay = docker_bay_h->getDockingBay();

	// Mostly the same as aicode.cpp: dock_orient_and_approach
	vec3d dockee_dock_pos, docker_dock_pos_local, docker_dock_pos;
	vec3d dock_up_dir;

	// Get the center between the two docking points
	vm_vec_avg(&dockee_dock_pos, &dockee_bay->pnt[0], &dockee_bay->pnt[1]);
	vm_vec_avg(&docker_dock_pos_local, &docker_bay->pnt[0], &docker_bay->pnt[1]);

	// Get the up-vector of the docking bay
	vm_vec_normalized_dir(&dock_up_dir, &dockee_bay->pnt[1], &dockee_bay->pnt[0]);

	// Compute the orientation
	vm_vector_2_matrix_norm(&final_orient, &dockee_bay->norm[0], &dock_up_dir, nullptr);

	// Rotate the docker position into the right orientation
	vm_vec_unrotate(&docker_dock_pos, &docker_dock_pos_local, &final_orient);

	// The docker vector points into the wrong direction, we need to scale it appropriately
	vm_vec_scale(&docker_dock_pos, -1.0f);

	// Now get the position of the other vessel
	vm_vec_add(&final_pos, &dockee_dock_pos, &docker_dock_pos);

	return ade_set_args(L, "oo", l_Vector.Set(final_pos),l_Matrix.Set(matrix_h(&final_orient)));
}

ADE_FUNC(isValid, l_Dockingbay, nullptr, "Detects whether is valid or not", "boolean", "true if valid, false otherwise")
{
	dockingbay_h* dbh = nullptr;

	if (!ade_get_args(L, "o", l_Dockingbay.GetPtr(&dbh)))
	{
		return ADE_RETURN_FALSE;
	}

	if (dbh == nullptr)
	{
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", dbh->isValid());
}

}
}

//
//

#include "globalincs/utility.h"

#include "modelinstance.h"
#include "object.h"
#include "prop.h"
#include "propclass.h"

#include "prop/prop.h"

namespace scripting::api {

//**********HANDLE: Prop
ADE_OBJ_DERIV(l_Prop, object_h, "prop", "Prop handle", l_Object);

ADE_VIRTVAR(Name,
	l_Prop,
	"string",
	"Prop name. This is the actual name of the prop.",
	"string",
	"Prop name, or empty string if handle is invalid")
{
	object_h* objh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Prop.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if (!objh->isValid())
		return ade_set_error(L, "s", "");

	prop* propp = prop_id_lookup(objh->objp()->instance);

	if (ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(propp->prop_name);
		strncpy(propp->prop_name, s, len);
		propp->prop_name[len - 1] = 0;
	}

	return ade_set_args(L, "s", propp->prop_name);
}

ADE_VIRTVAR(Class,
	l_Prop,
	"propclass",
	"Prop class",
	"propclass",
	"Prop class, or invalid propclass handle if prop handle is invalid")
{
	object_h* objh;
	int idx = -1;
	if (!ade_get_args(L, "o|o", l_Prop.GetPtr(&objh), l_Propclass.Get(&idx)))
		return ade_set_error(L, "o", l_Propclass.Set(-1));

	if (!objh->isValid())
		return ade_set_error(L, "o", l_Propclass.Set(-1));

	prop* propp = prop_id_lookup(objh->objp()->instance);

	if (ADE_SETTING_VAR && SCP_vector_inbounds(Prop_info, idx)) {
		change_prop_type(objh->objp()->instance, idx);
	}

	if (propp->prop_info_index < 0)
		return ade_set_error(L, "o", l_Propclass.Set(-1));

	return ade_set_args(L, "o", l_Propclass.Set(propp->prop_info_index));
}

ADE_VIRTVAR(Textures,
	l_Prop,
	"modelinstancetextures",
	"Gets prop textures",
	"modelinstancetextures",
	"Prop textures, or invalid proptextures handle if prop handle is invalid")
{
	object_h* sh = nullptr;
	object_h* dh;
	if (!ade_get_args(L, "o|o", l_Prop.GetPtr(&dh), l_Prop.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	if (!dh->isValid())
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	prop* propp = prop_id_lookup(dh->objp()->instance);

	polymodel_instance* dest = model_get_instance(propp->model_instance_num);

	if (ADE_SETTING_VAR && sh && sh->isValid()) {
		dest->texture_replace = model_get_instance(propp->model_instance_num)->texture_replace;
	}

	return ade_set_args(L, "o", l_ModelInstanceTextures.Set(modelinstance_h(dest)));
}

} // namespace scripting::api

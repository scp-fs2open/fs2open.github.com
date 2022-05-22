//
//

#include "decaldefinition.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/model.h"
#include "scripting/api/objs/vecmath.h"
#include "decals/decals.h"

namespace scripting {
namespace api {


//**********HANDLE: DecalDefinitionclass
ADE_OBJ(l_DecalDefinitionclass, int, "decaldefinition", "Decal definition handle");

ADE_FUNC(__tostring, l_DecalDefinitionclass, nullptr, "Decal definition name", "string", "Decal definition unique id, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_DecalDefinitionclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= static_cast<int>(decals::DecalDefinitions.size()))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", decals::DecalDefinitions[idx].getName());
}

ADE_FUNC(__eq, l_DecalDefinitionclass, "decaldefinition, decaldefinition", "Checks if the two definitions are equal", "boolean", "true if equal, false otherwise")
{
	int idx1,idx2;
	if (!ade_get_args(L, "oo", l_DecalDefinitionclass.Get(&idx1), l_DecalDefinitionclass.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if (idx1 < 0 || idx1 >= static_cast<int>(decals::DecalDefinitions.size()))
		return ade_set_error(L, "b", false);

	if (idx2 < 0 || idx2 >= static_cast<int>(decals::DecalDefinitions.size()))
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_FUNC(isValid, l_DecalDefinitionclass, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if invalid, nil if a syntax/type error occurs")
{
	int idx;
	if (!ade_get_args(L, "o", l_DecalDefinitionclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= static_cast<int>(decals::DecalDefinitions.size()))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(Name, l_DecalDefinitionclass, "string", "Decal definition name", "string", "Decal definition name, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_DecalDefinitionclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= static_cast<int>(decals::DecalDefinitions.size()))
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the decal definition name is not implemented");

	return ade_set_args(L, "s", decals::DecalDefinitions[idx].getName());
}

ADE_FUNC(create, l_DecalDefinitionclass, "number width, number height, number minLifetime, number maxLifetime, object host, submodel submodel, [vector local_pos, orientation local_orient]",
	"Creates a decal with the specified parameters.  A negative value for either lifetime will result in a perpetual decal.  The position and orientation are in the frame-of-reference of the submodel.", nullptr, "Nothing")
{
	int idx;
	float width, height, minLifetime, maxLifetime;
	object_h *objh = nullptr;
	submodel_h *smh = nullptr;
	vec3d *local_pos = nullptr;
	matrix_h *local_orient = nullptr;

	if (!ade_get_args(L, "offffoo|oo", l_DecalDefinitionclass.Get(&idx), &width, &height, &minLifetime, &maxLifetime, l_Object.GetPtr(&objh), l_Submodel.GetPtr(&smh), l_Vector.GetPtr(&local_pos), l_Matrix.GetPtr(&local_orient)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= static_cast<int>(decals::DecalDefinitions.size()))
		return ADE_RETURN_NIL;

	if (!objh->IsValid() || !smh->IsValid())
		return ADE_RETURN_NIL;

	decals::creation_info info;
	info.definition_handle = idx;
	info.width = width;
	info.height = height;

	if (minLifetime <= 0.0f || maxLifetime <= 0.0f)
		info.lifetime = util::UniformFloatRange(-1.0f);
	else if (minLifetime == maxLifetime)
		info.lifetime = util::UniformFloatRange(minLifetime);
	else
		info.lifetime = util::UniformFloatRange(minLifetime, maxLifetime);

	decals::addDecal(info, objh->objp, smh->GetSubmodelIndex(), local_pos == nullptr ? vmd_zero_vector : *local_pos, local_orient == nullptr ? vmd_identity_matrix : *local_orient->GetMatrix());

	return ADE_RETURN_NIL;
}

}
}

//
//

#include "texturemap.h"
#include "texture.h"

#include "bmpman/bmpman.h"

namespace scripting {
namespace api {

int model_find_texture_slot(polymodel *pm, const char *name)
{
	if (pm == nullptr || name == nullptr)
		return -1;

	for (int i = 0; i < pm->n_textures; i++)
	{
		int tnum = pm->maps[i].FindTexture(name);
		if (tnum >= 0)
			return i * TM_NUM_TYPES + tnum;
	}

	return -1;
}

int model_instance_find_texture_slot(polymodel_instance *pmi, polymodel *pm, const char *name)
{
	if (pmi == nullptr || name == nullptr)
		return -1;

	if (pmi->texture_replace != nullptr)
	{
		char fname[MAX_FILENAME_LEN];

		for (int i = 0; i < MAX_REPLACEMENT_TEXTURES; i++)
		{
			int handle = (*pmi->texture_replace)[i];
			if (handle < 0)
				continue;

			bm_get_filename(handle, fname);
			if (!strextcmp(fname, name))
				return i;
		}
	}

	return model_find_texture_slot(pm, name);
}


texture_map_h::texture_map_h()
	: model_num(-1), pmi_id(-1), map_index(-1)
{}
texture_map_h::texture_map_h(polymodel *pm, int n_map_index)
	: model_num(pm ? pm->id : -1), pmi_id(-1), map_index(n_map_index)
{}
texture_map_h::texture_map_h(polymodel_instance *pmi, int n_map_index)
	: model_num(pmi ? pmi->model_num : -1), pmi_id(pmi ? pmi->id : -1), map_index(n_map_index)
{}

polymodel *texture_map_h::GetModel() const
{
	return isValid() ? model_get(model_num) : nullptr;
}
polymodel_instance *texture_map_h::GetModelInstance() const
{
	return (isValid() && pmi_id >= 0) ? model_get_instance(pmi_id) : nullptr;
}
texture_map *texture_map_h::Get() const
{
	return isValid() ? &model_get(model_num)->maps[map_index] : nullptr;
}

int texture_map_h::GetModelID() const
{
	return model_num;
}
int texture_map_h::GetModelInstanceID() const
{
	return pmi_id;
}
int texture_map_h::GetIndex() const
{
	return map_index;
}

bool texture_map_h::isInstance() const
{
	return pmi_id >= 0;
}

int texture_map_h::GetSlotTexture(int slot) const
{
	if (!isValid() || slot < 0 || slot >= TM_NUM_TYPES)
		return -1;

	if (pmi_id >= 0)
	{
		auto pmi = model_get_instance(pmi_id);
		if (pmi->texture_replace != nullptr)
		{
			int replacement = (*pmi->texture_replace)[map_index * TM_NUM_TYPES + slot];
			if (replacement >= 0)
				return replacement;
		}
	}

	return model_get(model_num)->maps[map_index].textures[slot].GetTexture();
}

void texture_map_h::SetSlotTexture(int slot, int bm_handle)
{
	if (!isValid() || slot < 0 || slot >= TM_NUM_TYPES)
		return;

	if (pmi_id >= 0)
	{
		auto pmi = model_get_instance(pmi_id);
		if (pmi->texture_replace == nullptr)
			pmi->texture_replace = std::make_shared<model_texture_replace>();

		// an invalid handle clears the replacement
		pmi->texture_replace->reference(map_index * TM_NUM_TYPES + slot, bm_handle);
	}
	else
	{
		// an invalid handle blanks the slot
		model_get(model_num)->maps[map_index].textures[slot].SetTexture(bm_is_valid(bm_handle) ? bm_handle : -1, true);
	}
}

bool texture_map_h::isValid() const
{
	if (model_num < 0 || map_index < 0)
		return false;

	if (pmi_id >= 0)
	{
		if (pmi_id >= num_model_instances())
			return false;

		auto pmi = model_get_instance(pmi_id);
		if (pmi == nullptr || pmi->model_num != model_num)
			return false;
	}

	auto pm = model_get(model_num);
	return pm != nullptr && map_index < pm->n_textures;
}


//**********HANDLE: texture_map
ADE_OBJ(l_TextureMap, texture_map_h, "texture_map", "One material of a model: the set of " SCP_TOKEN_TO_STR(TM_NUM_TYPES) " texture slots (base, glow, specular, normal, height, misc, reflectance, ambient occlusion) that the model draws a group of polygons with.  Handles come from the TextureMaps array of a model or of a model instance (ship, prop, etc.).  A model-level handle reads and writes the shared model, and so affects every object using it.  An instance-level handle reads the instance's replacement texture for a slot if one is set (otherwise the model's texture) and writes replacement textures on that instance only.  The flat \"textures\" and \"modelinstancetextures\" handles expose the same slots as one array with " SCP_TOKEN_TO_STR(TM_NUM_TYPES) " entries per material.");

// Setter documentation shared by every slot accessor
#define TEXTURE_MAP_SET_DOC "Setting a model-level handle changes the shared model; setting an instance-level handle sets a replacement texture on that instance only.  Assign an invalid texture handle (not nil, which does nothing) to blank the slot at model level or to clear the replacement at instance level.  The model or instance takes its own reference to the texture, so the script does not need to keep the handle alive."

// Gets or sets one slot of a texture map; the caller has already parsed the arguments
static int texture_map_slot(lua_State *L, texture_map_h *tmh, int slot, texture_h *new_tex)
{
	if (tmh == nullptr || !tmh->isValid())
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (ADE_SETTING_VAR && new_tex != nullptr)
		tmh->SetSlotTexture(slot, new_tex->handle);

	return ade_set_args(L, "o", l_Texture.Set(texture_h(tmh->GetSlotTexture(slot))));
}

// Parses the arguments of a slot virtvar and gets or sets that slot
static int texture_map_slot_virtvar(lua_State *L, int slot)
{
	texture_map_h *tmh = nullptr;
	texture_h *new_tex = nullptr;
	if (!ade_get_args(L, "o|o", l_TextureMap.GetPtr(&tmh), l_Texture.GetPtr(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	return texture_map_slot(L, tmh, slot, new_tex);
}

ADE_VIRTVAR(BaseMap, l_TextureMap, "texture", "Base (diffuse) texture.  " TEXTURE_MAP_SET_DOC, "texture", "Base texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_BASE_TYPE);
}

ADE_VIRTVAR(GlowMap, l_TextureMap, "texture", "Glow texture (-glow).  " TEXTURE_MAP_SET_DOC, "texture", "Glow texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_GLOW_TYPE);
}

ADE_VIRTVAR(SpecularMap, l_TextureMap, "texture", "Specular texture (-shine).  " TEXTURE_MAP_SET_DOC, "texture", "Specular texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_SPECULAR_TYPE);
}

ADE_VIRTVAR(NormalMap, l_TextureMap, "texture", "Normal texture (-normal).  " TEXTURE_MAP_SET_DOC, "texture", "Normal texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_NORMAL_TYPE);
}

ADE_VIRTVAR(HeightMap, l_TextureMap, "texture", "Height texture (-height), used for parallax mapping.  " TEXTURE_MAP_SET_DOC, "texture", "Height texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_HEIGHT_TYPE);
}

ADE_VIRTVAR(MiscMap, l_TextureMap, "texture", "Misc (utility) texture (-misc).  " TEXTURE_MAP_SET_DOC, "texture", "Misc texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_MISC_TYPE);
}

ADE_VIRTVAR(ReflectanceMap, l_TextureMap, "texture", "Reflectance texture (-reflect), holding specular and gloss.  " TEXTURE_MAP_SET_DOC, "texture", "Reflectance texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_SPEC_GLOSS_TYPE);
}

ADE_VIRTVAR(AmbientOcclusionMap, l_TextureMap, "texture", "Ambient occlusion texture (-ao).  " TEXTURE_MAP_SET_DOC, "texture", "Ambient occlusion texture, or invalid texture handle if the handle is invalid")
{
	return texture_map_slot_virtvar(L, TM_AMBIENT_TYPE);
}

ADE_INDEXER(l_TextureMap, "number Slot", "Gets or sets a texture slot by 1-based slot number, in the order base, glow, specular, normal, height, misc, reflectance, ambient occlusion (the same order the flat \"textures\" handle uses within each material).  " TEXTURE_MAP_SET_DOC, "texture", "Texture, or invalid texture handle if the handle or the slot number is invalid")
{
	texture_map_h *tmh = nullptr;
	int slot = -1;
	texture_h *new_tex = nullptr;
	if (!ade_get_args(L, "oi|o", l_TextureMap.GetPtr(&tmh), &slot, l_Texture.GetPtr(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (slot < 1 || slot > TM_NUM_TYPES)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	return texture_map_slot(L, tmh, slot - 1, new_tex);
}

ADE_FUNC(__len, l_TextureMap, nullptr, "Number of texture slots in a material, i.e. " SCP_TOKEN_TO_STR(TM_NUM_TYPES), "number", "Number of slots, or 0 if handle is invalid")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ade_set_error(L, "i", 0);

	if (!tmh->isValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", TM_NUM_TYPES);
}

ADE_FUNC(__eq, l_TextureMap, "texture_map, texture_map", "Checks if two handles refer to the same material of the same model or model instance", "boolean", "True if the handles are equal")
{
	texture_map_h *tmh1;
	texture_map_h *tmh2;

	if (!ade_get_args(L, "oo", l_TextureMap.GetPtr(&tmh1), l_TextureMap.GetPtr(&tmh2)))
		return ADE_RETURN_NIL;

	if (tmh1->GetModelID() == tmh2->GetModelID() && tmh1->GetModelInstanceID() == tmh2->GetModelInstanceID() && tmh1->GetIndex() == tmh2->GetIndex())
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Index, l_TextureMap, nullptr, "1-based index of this material in the TextureMaps array.  The material's first entry in the flat \"textures\" array is at (Index-1)*" SCP_TOKEN_TO_STR(TM_NUM_TYPES) "+1.", "number", "Index, or 0 if handle is invalid")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ade_set_error(L, "i", 0);

	if (!tmh->isValid())
		return ade_set_error(L, "i", 0);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "i", tmh->GetIndex() + 1);
}

ADE_VIRTVAR(IsTransparent, l_TextureMap, nullptr, "Whether the model marks this material as transparent (-trans)", "boolean", "true if transparent, false if not or if handle is invalid")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ade_set_error(L, "b", false);

	texture_map *tmap = tmh->Get();
	if (tmap == nullptr)
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "b", tmap->is_transparent);
}

ADE_VIRTVAR(IsAmbient, l_TextureMap, nullptr, "Whether this material uses an ambient (self-illuminated) shader", "boolean", "true if ambient, false if not or if handle is invalid")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ade_set_error(L, "b", false);

	texture_map *tmap = tmh->Get();
	if (tmap == nullptr)
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "b", tmap->is_ambient);
}

ADE_FUNC(resetToOriginal, l_TextureMap, nullptr, "Model-level handles only: restores every slot of this material to the texture loaded from the model file, releasing any references taken by script assignments.  Affects every object using the model.", "boolean", "true if reset, false if the handle is invalid or is an instance-level handle")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ADE_RETURN_NIL;

	texture_map *tmap = tmh->Get();
	if (tmap == nullptr || tmh->isInstance())
		return ADE_RETURN_FALSE;

	tmap->ResetToOriginal();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetToModel, l_TextureMap, nullptr, "Instance-level handles only: clears the " SCP_TOKEN_TO_STR(TM_NUM_TYPES) " replacement textures of this material, so that the instance draws the model's textures again.  Note that instances which were assigned each other's Textures share one set of replacement textures, so this affects those instances too.", "boolean", "true if cleared, false if the handle is invalid or is a model-level handle")
{
	texture_map_h *tmh = nullptr;
	if (!ade_get_args(L, "o", l_TextureMap.GetPtr(&tmh)))
		return ADE_RETURN_NIL;

	polymodel_instance *pmi = tmh->GetModelInstance();
	if (pmi == nullptr)
		return ADE_RETURN_FALSE;

	if (pmi->texture_replace != nullptr)
	{
		for (int i = 0; i < TM_NUM_TYPES; i++)
			pmi->texture_replace->clear(tmh->GetIndex() * TM_NUM_TYPES + i);
	}

	return ADE_RETURN_TRUE;
}


//**********HANDLE: texturemaps
ADE_OBJ(l_ModelTextureMaps, model_h, "texturemaps", "Array of a model's materials, as model-level texture_map handles.  Entry N corresponds to entries (N-1)*" SCP_TOKEN_TO_STR(TM_NUM_TYPES) "+1 through N*" SCP_TOKEN_TO_STR(TM_NUM_TYPES) " of the model's flat \"textures\" handle.");

ADE_FUNC(__len, l_ModelTextureMaps, nullptr, "Number of materials on the model", "number", "Number of materials, or 0 if handle is invalid")
{
	model_h *mdl = nullptr;
	if (!ade_get_args(L, "o", l_ModelTextureMaps.GetPtr(&mdl)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = mdl->Get();
	if (pm == nullptr)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_textures);
}

ADE_INDEXER(l_ModelTextureMaps, "number/string IndexOrTextureFilename", "Gets a material by 1-based index, or by the filename of the texture in any of its slots", "texture_map", "Material handle, or invalid texture_map handle if the model handle is invalid or nothing matches")
{
	model_h *mdl = nullptr;
	int index = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi", l_ModelTextureMaps.GetPtr(&mdl), &index))
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		index--;	// Lua -> FS2
	}
	else
	{
		const char *name = nullptr;
		if (!ade_get_args(L, "os", l_ModelTextureMaps.GetPtr(&mdl), &name))
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		int slot = model_find_texture_slot(mdl->Get(), name);
		if (slot < 0)
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		index = slot / TM_NUM_TYPES;
	}

	polymodel *pm = mdl->Get();
	if (pm == nullptr || index < 0 || index >= pm->n_textures)
		return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning texture maps is not supported");

	return ade_set_args(L, "o", l_TextureMap.Set(texture_map_h(pm, index)));
}


//**********HANDLE: modelinstancetexturemaps
ADE_OBJ(l_ModelInstanceTextureMaps, modelinstance_h, "modelinstancetexturemaps", "Array of a model instance's materials, as instance-level texture_map handles.  It has the same layout as the model's \"texturemaps\" array; see also the flat \"modelinstancetextures\" handle.");

ADE_FUNC(__len, l_ModelInstanceTextureMaps, nullptr, "Number of materials on the model instance", "number", "Number of materials, or 0 if handle is invalid")
{
	modelinstance_h *mih = nullptr;
	if (!ade_get_args(L, "o", l_ModelInstanceTextureMaps.GetPtr(&mih)))
		return ade_set_error(L, "i", 0);

	polymodel *pm = mih->GetModel();
	if (pm == nullptr)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_textures);
}

ADE_INDEXER(l_ModelInstanceTextureMaps, "number/string IndexOrTextureFilename", "Gets a material by 1-based index, or by texture filename.  A filename is matched first against this instance's replacement textures, then against the texture in any slot of any material of the model.", "texture_map", "Material handle, or invalid texture_map handle if the model instance handle is invalid or nothing matches")
{
	modelinstance_h *mih = nullptr;
	int index = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "oi", l_ModelInstanceTextureMaps.GetPtr(&mih), &index))
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		index--;	// Lua -> FS2
	}
	else
	{
		const char *name = nullptr;
		if (!ade_get_args(L, "os", l_ModelInstanceTextureMaps.GetPtr(&mih), &name))
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		int slot = model_instance_find_texture_slot(mih->Get(), mih->GetModel(), name);
		if (slot < 0)
			return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

		index = slot / TM_NUM_TYPES;
	}

	polymodel_instance *pmi = mih->Get();
	polymodel *pm = mih->GetModel();
	if (pmi == nullptr || pm == nullptr || index < 0 || index >= pm->n_textures)
		return ade_set_error(L, "o", l_TextureMap.Set(texture_map_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Assigning texture maps is not supported");

	return ade_set_args(L, "o", l_TextureMap.Set(texture_map_h(pmi, index)));
}

}
}

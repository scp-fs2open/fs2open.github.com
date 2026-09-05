//
//

#include "texture_replacement.h"

namespace scripting {
namespace api {

texture_replacement_h::texture_replacement_h()
	: m_ship_info_idx(-1), m_index(INVALID_ID)
{}
texture_replacement_h::texture_replacement_h(int ship_info_idx, size_t index)
	: m_ship_info_idx(ship_info_idx), m_index(index)
{}
const texture_replace *texture_replacement_h::Get() const
{
	return isValid() ? &Ship_info[m_ship_info_idx].replacement_textures[m_index] : nullptr;
}
bool texture_replacement_h::isValid() const
{
	return Ship_info.in_bounds(m_ship_info_idx) && m_index < Ship_info[m_ship_info_idx].replacement_textures.size();
}


//**********HANDLE: texture_replacement
ADE_OBJ(l_TextureReplacement, texture_replacement_h, "texture_replacement", "One replacement texture of a ship class, as defined in its table entry: the filename of a model texture and the filename of the texture that replaces it on every ship of the class.  Only the filenames are available here; the replacement bitmaps are loaded for each ship when it is created, and can be inspected through the ship's Textures or TextureMaps.");

ADE_VIRTVAR(OldFilename, l_TextureReplacement, nullptr, "Filename of the model texture that is replaced", "string", "Filename, or empty string if handle is invalid")
{
	texture_replacement_h *trh = nullptr;
	if (!ade_get_args(L, "o", l_TextureReplacement.GetPtr(&trh)))
		return ade_set_error(L, "s", "");

	auto tr = trh->Get();
	if (tr == nullptr)
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "s", tr->old_texture);
}

ADE_VIRTVAR(NewFilename, l_TextureReplacement, nullptr, "Filename of the texture that replaces it", "string", "Filename, or empty string if handle is invalid")
{
	texture_replacement_h *trh = nullptr;
	if (!ade_get_args(L, "o", l_TextureReplacement.GetPtr(&trh)))
		return ade_set_error(L, "s", "");

	auto tr = trh->Get();
	if (tr == nullptr)
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "s", tr->new_texture);
}


shipclass_texture_replacements_h::shipclass_texture_replacements_h()
	: m_ship_info_idx(-1)
{}
shipclass_texture_replacements_h::shipclass_texture_replacements_h(int ship_info_idx)
	: m_ship_info_idx(ship_info_idx)
{}
const ship_info *shipclass_texture_replacements_h::GetShipInfoPtr() const
{
	return isValid() ? &Ship_info[m_ship_info_idx] : nullptr;
}
int shipclass_texture_replacements_h::GetShipInfoIndex() const
{
	return isValid() ? m_ship_info_idx : -1;
}
bool shipclass_texture_replacements_h::isValid() const
{
	return Ship_info.in_bounds(m_ship_info_idx);
}


//**********HANDLE: texture_replacements
ADE_OBJ(l_ShipclassTextureReplacements, shipclass_texture_replacements_h, "texture_replacements", "Array of a ship class's replacement textures");

ADE_FUNC(__len, l_ShipclassTextureReplacements, nullptr, "Number of replacement textures defined for the ship class", "number", "Number of replacement textures, or 0 if handle is invalid")
{
	shipclass_texture_replacements_h *trh = nullptr;
	if (!ade_get_args(L, "o", l_ShipclassTextureReplacements.GetPtr(&trh)))
		return ade_set_error(L, "i", 0);

	auto sip = trh->GetShipInfoPtr();
	if (sip == nullptr)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", static_cast<int>(sip->replacement_textures.size()));
}

ADE_INDEXER(l_ShipclassTextureReplacements, "number/string IndexOrOldFilename", "Gets a replacement texture by 1-based index, or by the filename of the model texture it replaces", "texture_replacement", "Replacement texture handle, or invalid texture_replacement handle if the handle is invalid or nothing matches")
{
	shipclass_texture_replacements_h *trh = nullptr;
	size_t index = INVALID_ID;

	if (lua_isnumber(L, 2))
	{
		int lua_index = -1;
		if (!ade_get_args(L, "oi", l_ShipclassTextureReplacements.GetPtr(&trh), &lua_index))
			return ade_set_error(L, "o", l_TextureReplacement.Set(texture_replacement_h()));

		if (lua_index < 1)
			return ade_set_error(L, "o", l_TextureReplacement.Set(texture_replacement_h()));

		index = static_cast<size_t>(lua_index - 1);	// Lua -> FS2
	}
	else
	{
		const char *name = nullptr;
		if (!ade_get_args(L, "os", l_ShipclassTextureReplacements.GetPtr(&trh), &name))
			return ade_set_error(L, "o", l_TextureReplacement.Set(texture_replacement_h()));

		auto sip = trh->GetShipInfoPtr();
		if (sip == nullptr || name == nullptr)
			return ade_set_error(L, "o", l_TextureReplacement.Set(texture_replacement_h()));

		for (size_t i = 0; i < sip->replacement_textures.size(); i++)
		{
			if (!strextcmp(sip->replacement_textures[i].old_texture, name))
			{
				index = i;
				break;
			}
		}
	}

	auto sip = trh->GetShipInfoPtr();
	if (sip == nullptr || index >= sip->replacement_textures.size())
		return ade_set_error(L, "o", l_TextureReplacement.Set(texture_replacement_h()));

	if (ADE_SETTING_VAR)
		LuaError(L, "Replacement textures are read only.");

	return ade_set_args(L, "o", l_TextureReplacement.Set(texture_replacement_h(trh->GetShipInfoIndex(), index)));
}

}
}

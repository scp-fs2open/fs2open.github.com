//
//

#include "texturemap.h"
#include "texture.h"

namespace scripting {
namespace api {

texture_map_h::texture_map_h() {
	type = THT_INDEPENDENT;
	tmap = NULL;
}
texture_map_h::texture_map_h(object* objp, texture_map* n_tmap) {
	type = THT_OBJECT;
	obj = object_h(objp);
	tmap = n_tmap;
}
texture_map_h::texture_map_h(int modelnum, texture_map* n_tmap) {
	type = THT_MODEL;
	mdl = model_h(modelnum);
	tmap = n_tmap;
}
texture_map_h::texture_map_h(polymodel* n_model, texture_map* n_tmap) {
	type = THT_MODEL;
	mdl = model_h(n_model);
	tmap = n_tmap;
}
texture_map* texture_map_h::Get() {
	if(!this->IsValid())
		return NULL;

	return tmap;
}
int texture_map_h::GetSize() {
	if(!this->IsValid())
		return 0;

	switch(type)
	{
		case THT_MODEL:
			return mdl.Get()->n_textures;
		case THT_OBJECT:
			return 0;	//Can't do this right now.
		default:
			return 0;
	}
}
bool texture_map_h::IsValid() {
	if(tmap == NULL)
		return false;

	switch(type)
	{
		case THT_INDEPENDENT:
			return true;
		case THT_OBJECT:
			return obj.IsValid();
		case THT_MODEL:
			return mdl.IsValid();
		default:
			Error(LOCATION, "Bad type in texture_map_h; debug this.");
			return false;
	}
}

ADE_OBJ(l_TextureMap, texture_map_h, "material", "Texture map, including diffuse, glow, and specular textures");

ADE_VIRTVAR(BaseMap, l_TextureMap, "texture", "Base texture", "texture", "Base texture, or invalid texture handle if material handle is invalid")
{
	texture_map_h *tmh = NULL;
	int new_tex = -1;
	if(!ade_get_args(L, "o|o", l_TextureMap.GetPtr(&tmh), l_Texture.Get(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	texture_map *tmap = tmh->Get();
	if(tmap == NULL)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(ADE_SETTING_VAR && new_tex > -1) {
		tmap->textures[TM_BASE_TYPE].SetTexture(new_tex);
	}

	return ade_set_args(L, "o", l_Texture.Set(tmap->textures[TM_BASE_TYPE].GetTexture()));
}

ADE_VIRTVAR(GlowMap, l_TextureMap, "texture", "Glow texture", "texture", "Glow texture, or invalid texture handle if material handle is invalid")
{
	texture_map_h *tmh = NULL;
	int new_tex = -1;
	if(!ade_get_args(L, "o|o", l_TextureMap.GetPtr(&tmh), l_Texture.Get(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	texture_map *tmap = tmh->Get();
	if(tmap == NULL)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(ADE_SETTING_VAR && new_tex > -1) {
		tmap->textures[TM_GLOW_TYPE].SetTexture(new_tex);
	}

	return ade_set_args(L, "o", l_Texture.Set(tmap->textures[TM_GLOW_TYPE].GetTexture()));
}

ADE_VIRTVAR(SpecularMap, l_TextureMap, "texture", "Specular texture", "texture", "Texture handle, or invalid texture handle if material handle is invalid")
{
	texture_map_h *tmh = NULL;
	int new_tex = -1;
	if(!ade_get_args(L, "o|o", l_TextureMap.GetPtr(&tmh), l_Texture.Get(&new_tex)))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	texture_map *tmap = tmh->Get();
	if(tmap == NULL)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(ADE_SETTING_VAR && new_tex > -1) {
		tmap->textures[TM_SPECULAR_TYPE].SetTexture(new_tex);
	}

	return ade_set_args(L, "o", l_Texture.Set(tmap->textures[TM_SPECULAR_TYPE].GetTexture()));
}

}
}

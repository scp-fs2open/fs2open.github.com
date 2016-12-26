//
//

#include "ship.h"
#include "texture.h"

#include "object/object.h"
#include "model/model.h"
#include "ship/ship.h"
#include "parse/parselo.h"

namespace scripting {
namespace api {

//**********HANDLE: shiptextures
ADE_OBJ(l_ShipTextures, object_h, "shiptextures", "Ship textures handle");

ADE_FUNC(__len, l_ShipTextures, NULL, "Number of textures on ship", "number", "Number of textures on ship, or 0 if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_ShipTextures.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	polymodel *pm = model_get(Ship_info[Ships[objh->objp->instance].ship_info_index].model_num);

	if(pm == NULL)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", pm->n_textures*TM_NUM_TYPES);
}

ADE_INDEXER(l_ShipTextures, "number Index/string TextureFilename", "Array of ship textures", "texture", "Texture, or invalid texture handle on failure")
{
	object_h *oh;
	char *s;
	int tdx=-1;
	if (!ade_get_args(L, "os|o", l_ShipTextures.GetPtr(&oh), &s, l_Texture.Get(&tdx)))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if (!oh->IsValid() || s==NULL)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	ship *shipp = &Ships[oh->objp->instance];
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);
	int final_index = -1;
	int i;

	char fname[MAX_FILENAME_LEN];
	if (shipp->ship_replacement_textures != NULL)
	{
		for(i = 0; i < MAX_REPLACEMENT_TEXTURES; i++)
		{
			bm_get_filename(shipp->ship_replacement_textures[i], fname);

			if(!strextcmp(fname, s)) {
				final_index = i;
				break;
			}
		}
	}

	if(final_index < 0)
	{
		for (i = 0; i < pm->n_textures; i++)
		{
			int tm_num = pm->maps[i].FindTexture(s);
			if(tm_num > -1)
			{
				final_index = i*TM_NUM_TYPES+tm_num;
				break;
			}
		}
	}

	if (final_index < 0)
	{
		final_index = atoi(s) - 1;	//Lua->FS2

		if (final_index < 0 || final_index >= MAX_REPLACEMENT_TEXTURES)
			return ade_set_error(L, "o", l_Texture.Set(-1));
	}

	if (ADE_SETTING_VAR) {
		if (shipp->ship_replacement_textures == NULL) {
			shipp->ship_replacement_textures = (int *) vm_malloc(MAX_REPLACEMENT_TEXTURES * sizeof(int));

			for (i = 0; i < MAX_REPLACEMENT_TEXTURES; i++)
				shipp->ship_replacement_textures[i] = -1;
		}

		if(bm_is_valid(tdx))
			shipp->ship_replacement_textures[final_index] = tdx;
		else
			shipp->ship_replacement_textures[final_index] = -1;
	}

	if (shipp->ship_replacement_textures != NULL && shipp->ship_replacement_textures[final_index] >= 0)
		return ade_set_args(L, "o", l_Texture.Set(shipp->ship_replacement_textures[final_index]));
	else
		return ade_set_args(L, "o", l_Texture.Set(pm->maps[final_index / TM_NUM_TYPES].textures[final_index % TM_NUM_TYPES].GetTexture()));
}

ADE_FUNC(isValid, l_ShipTextures, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_ShipTextures.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", oh->IsValid());
}


}
}

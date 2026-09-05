#pragma once

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include "model.h"
#include "modelinstance.h"
#include "model/model.h"

namespace scripting {
namespace api {

// Finds the texture with the given filename on a model.  Returns the flat slot index (texture_map index * TM_NUM_TYPES + slot type),
// as used by the "textures" Lua handle, or -1 if not found.
int model_find_texture_slot(polymodel *pm, const char *name);

// As above, but checks the instance's replacement textures first, as the "modelinstancetextures" Lua handle does.
int model_instance_find_texture_slot(polymodel_instance *pmi, polymodel *pm, const char *name);

// A handle to one texture_map (material) of a model, or of a model instance.  A model-level handle reads and writes the
// polymodel's own texture_info slots; an instance-level handle reads and writes the instance's replacement textures.
class texture_map_h
{
 protected:
	int model_num;		// polymodel::id, as in model_h
	int pmi_id;			// polymodel_instance::id, or -1 for a model-level handle
	int map_index;		// index into polymodel::maps

 public:
	texture_map_h();
	explicit texture_map_h(polymodel *pm, int n_map_index);
	explicit texture_map_h(polymodel_instance *pmi, int n_map_index);

	polymodel *GetModel() const;
	polymodel_instance *GetModelInstance() const;	// nullptr for a model-level handle
	texture_map *Get() const;

	int GetModelID() const;
	int GetModelInstanceID() const;					// -1 for a model-level handle
	int GetIndex() const;							// 0-based

	bool isInstance() const;

	// These follow the same rules as the flat "textures" and "modelinstancetextures" Lua handles: at model level they read and
	// write the shared model (taking a reference to the texture); at instance level they read the replacement texture if one
	// is set (otherwise the model's texture) and write replacement textures.  Setting an invalid handle blanks the slot at
	// model level and clears the replacement at instance level.
	int GetSlotTexture(int slot) const;
	void SetSlotTexture(int slot, int bm_handle);

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_TextureMap, texture_map_h);

DECLARE_ADE_OBJ(l_ModelTextureMaps, model_h);
DECLARE_ADE_OBJ(l_ModelInstanceTextureMaps, modelinstance_h);

}
}

#pragma once

#include "scripting/ade_api.h"
#include "mission/missionparse.h"
#include "ship/ship.h"

namespace scripting {
namespace api {

// One replacement texture of a ship class, as defined in the class's table entry
class texture_replacement_h
{
 private:
	int m_ship_info_idx;
	size_t m_index;

 public:
	texture_replacement_h();
	explicit texture_replacement_h(int ship_info_idx, size_t index);

	const texture_replace *Get() const;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_TextureReplacement, texture_replacement_h);


// The array of a ship class's replacement textures
class shipclass_texture_replacements_h
{
 private:
	int m_ship_info_idx;

 public:
	shipclass_texture_replacements_h();
	explicit shipclass_texture_replacements_h(int ship_info_idx);

	const ship_info *GetShipInfoPtr() const;
	int GetShipInfoIndex() const;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_ShipclassTextureReplacements, shipclass_texture_replacements_h);

}
}

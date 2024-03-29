#pragma once

#include <object/object.h>
#include <ship/ship.h>
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

const int SWH_NONE = 0;
const int SWH_PRIMARY = 1;
const int SWH_SECONDARY = 2;
const int SWH_TERTIARY = 3;

struct ship_banktype_h
{
	object_h objh;
	ship_weapon *sw;
	int type;

	ship_banktype_h();
	ship_banktype_h(object *objp_in, ship_weapon *wpn, int in_type);

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_WeaponBankType, ship_banktype_h);

struct ship_bank_h
{
	ship_banktype_h typeh;
	int bank;

	ship_bank_h();
	ship_bank_h(object *objp_in, ship_weapon *wpn, int in_type, int in_bank);

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_WeaponBank, ship_bank_h);


}
}


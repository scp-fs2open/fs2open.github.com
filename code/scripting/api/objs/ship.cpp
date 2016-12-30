//
//


#include "ship.h"
#include "texture.h"
#include "object.h"
#include "subsystem.h"
#include "shipclass.h"
#include "cockpit_display.h"
#include "weaponclass.h"
#include "ship_bank.h"
#include "team.h"
#include "order.h"
#include "enums.h"
#include "wing.h"

#include "ship/shiphit.h"
#include "hud/hudshield.h"
#include "playerman/player.h"
#include "mission/missionlog.h"
#include "ai/aigoals.h"
#include "ship/shipfx.h"
#include "hud/hudets.h"
#include "object/object.h"
#include "model/model.h"
#include "ship/ship.h"
#include "parse/parselo.h"

extern void ship_reset_disabled_physics(object *objp, int ship_class);

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

//**********HANDLE: Ship
ADE_OBJ_DERIV(l_Ship, object_h, "ship", "Ship handle", l_Object);

ADE_INDEXER(l_Ship, "string Name/number Index", "Array of ship subsystems", "subsystem", "Subsystem handle, or invalid subsystem handle if index or ship handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	ship_subsys_h *sub = nullptr;
	if(!ade_get_args(L, "o|so", l_Ship.GetPtr(&objh), &s, l_Subsystem.GetPtr(&sub)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ship *shipp = &Ships[objh->objp->instance];
	ship_subsys *ss = ship_get_subsys(shipp, s);

	if(ss == NULL)
	{
		int idx = atoi(s);
		if(idx > 0 && idx <= ship_get_num_subsys(shipp))
		{
			idx--; //Lua->FS2
			ss = ship_get_indexed_subsys(shipp, idx);
		}
	}

	if(ss == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objh->objp, ss)));
}

ADE_FUNC(__len, l_Ship, NULL, "Number of subsystems on ship", "number", "Subsystem number, or 0 if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_get_num_subsys(&Ships[objh->objp->instance]));
}

ADE_VIRTVAR(ShieldArmorClass, l_Ship, "string", "Current Armor class of the ships' shield", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	char *s = NULL;
	char *name = NULL;

	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];
	int atindex = -1;
	if (ADE_SETTING_VAR && s != NULL) {
		atindex = armor_type_get_idx(s);
		shipp->shield_armor_type_idx = atindex;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(ArmorClass, l_Ship, "string", "Current Armor class", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	char *s = NULL;
	char *name = NULL;

	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];
	int atindex = -1;
	if (ADE_SETTING_VAR && s != NULL) {
		atindex = armor_type_get_idx(s);
		shipp->armor_type_idx = atindex;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(Name, l_Ship, "string", "Ship name", "string", "Ship name, or empty string if handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && s != NULL) {
		strncpy(shipp->ship_name, s, sizeof(shipp->ship_name)-1);
	}

	return ade_set_args(L, "s", shipp->ship_name);
}

ADE_VIRTVAR(AfterburnerFuelLeft, l_Ship, "number", "Afterburner fuel left", "number", "Afterburner fuel left, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		shipp->afterburner_fuel = fuel;

	return ade_set_args(L, "f", shipp->afterburner_fuel);
}

ADE_VIRTVAR(AfterburnerFuelMax, l_Ship, "number", "Afterburner fuel capacity", "number", "Afterburner fuel capacity, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp->instance].ship_info_index];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		sip->afterburner_fuel_capacity = fuel;

	return ade_set_args(L, "f", sip->afterburner_fuel_capacity);
}

ADE_VIRTVAR(Class, l_Ship, "shipclass", "Ship class", "shipclass", "Ship class, or invalid shipclass handle if ship handle is invalid")
{
	object_h *objh;
	int idx=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Shipclass.Get(&idx)))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && idx > -1) {
		change_ship_type(objh->objp->instance, idx, 1);
		if (shipp == Player_ship) {
			set_current_hud();
		}
	}

	if(shipp->ship_info_index < 0)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	return ade_set_args(L, "o", l_Shipclass.Set(shipp->ship_info_index));
}

ADE_VIRTVAR(CountermeasuresLeft, l_Ship, "number", "Number of countermeasures left", "number", "Countermeasures left, or 0 if ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &newcm))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && newcm > -1)
		shipp->cmeasure_count = newcm;

	return ade_set_args(L, "i", shipp->cmeasure_count);
}

ADE_VIRTVAR(CockpitDisplays, l_Ship, "displays", "An array of the cockpit displays on this ship.<br>NOTE: Only the ship of the player has these", "displays", "displays handle or invalid handle on error")
{
	object_h *objh;
	cockpit_displays_h *cdh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_CockpitDisplays.GetPtr(&cdh)))
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Cockpit displays copy");
	}

	return ade_set_args(L, "o", l_CockpitDisplays.Set(cockpit_displays_h(objh->objp)));
}

ADE_VIRTVAR(CountermeasureClass, l_Ship, "weaponclass", "Weapon class mounted on this ship's countermeasure point", "weaponclass", "Countermeasure hardpoint weapon class, or invalid weaponclass handle if no countermeasure class or ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Weaponclass.Get(&newcm)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR) {
		shipp->current_cmeasure = newcm;
	}

	if(shipp->current_cmeasure > -1)
		return ade_set_args(L, "o", l_Weaponclass.Set(shipp->current_cmeasure));
	else
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;
}

ADE_VIRTVAR(HitpointsMax, l_Ship, "number", "Total hitpoints", "number", "Ship maximum hitpoints, or 0 if handle is invalid")
{
	object_h *objh;
	float newhits = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &newhits))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && newhits > -1)
		shipp->ship_max_hull_strength = newhits;

	return ade_set_args(L, "f", shipp->ship_max_hull_strength);
}

ADE_VIRTVAR(WeaponEnergyLeft, l_Ship, "number", "Current weapon energy reserves", "number", "Ship current weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && neweng > -1)
		shipp->weapon_energy = neweng;

	return ade_set_args(L, "f", shipp->weapon_energy);
}

ADE_VIRTVAR(WeaponEnergyMax, l_Ship, "number", "Maximum weapon energy", "number", "Ship maximum weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp->instance].ship_info_index];

	if(ADE_SETTING_VAR && neweng > -1)
		sip->max_weapon_reserve = neweng;

	return ade_set_args(L, "f", sip->max_weapon_reserve);
}

ADE_VIRTVAR(AutoaimFOV, l_Ship, "number", "FOV of ship's autoaim, if any", "number", "FOV (in degrees), or 0 if ship uses no autoaim or if handle is invalid")
{
	object_h *objh;
	float fov = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fov))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && fov >= 0.0f) {
		if (fov > 180.0)
			fov = 180.0;

		shipp->autoaim_fov = fov * PI / 180.0f;
	}

	return ade_set_args(L, "f", shipp->autoaim_fov * 180.0f / PI);
}

ADE_VIRTVAR(PrimaryTriggerDown, l_Ship, "boolean", "Determines if primary trigger is pressed or not", "boolean", "True if pressed, false if not, nil if ship handle is invalid")
{
	object_h *objh;
	bool trig = false;
	if(!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &trig))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		if(trig)
			shipp->flags.set(Ship::Ship_Flags::Trigger_down);
		else
			shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
	}

	if (shipp->flags[Ship::Ship_Flags::Trigger_down])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}


ADE_VIRTVAR(PrimaryBanks, l_Ship, "weaponbanktype", "Array of primary weapon banks", "weaponbanktype", "Primary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_primary_bank = src->current_primary_bank;
		dst->num_primary_banks = src->num_primary_banks;

		memcpy(dst->next_primary_fire_stamp, src->next_primary_fire_stamp, sizeof(dst->next_primary_fire_stamp));
		memcpy(dst->primary_animation_done_time, src->primary_animation_done_time, sizeof(dst->primary_animation_done_time));
		memcpy(dst->primary_animation_position, src->primary_animation_position, sizeof(dst->primary_animation_position));
		memcpy(dst->primary_bank_ammo, src->primary_bank_ammo, sizeof(dst->primary_bank_ammo));
		memcpy(dst->primary_bank_capacity, src->primary_bank_capacity, sizeof(dst->primary_bank_capacity));
		memcpy(dst->primary_bank_rearm_time, src->primary_bank_rearm_time, sizeof(dst->primary_bank_rearm_time));
		memcpy(dst->primary_bank_start_ammo, src->primary_bank_start_ammo, sizeof(dst->primary_bank_start_ammo));
		memcpy(dst->primary_bank_weapons, src->primary_bank_weapons, sizeof(dst->primary_bank_weapons));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_PRIMARY)));
}

ADE_VIRTVAR(SecondaryBanks, l_Ship, "weaponbanktype", "Array of secondary weapon banks", "weaponbanktype", "Secondary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_secondary_bank = src->current_secondary_bank;
		dst->num_secondary_banks = src->num_secondary_banks;

		memcpy(dst->next_secondary_fire_stamp, src->next_secondary_fire_stamp, sizeof(dst->next_secondary_fire_stamp));
		memcpy(dst->secondary_animation_done_time, src->secondary_animation_done_time, sizeof(dst->secondary_animation_done_time));
		memcpy(dst->secondary_animation_position, src->secondary_animation_position, sizeof(dst->secondary_animation_position));
		memcpy(dst->secondary_bank_ammo, src->secondary_bank_ammo, sizeof(dst->secondary_bank_ammo));
		memcpy(dst->secondary_bank_capacity, src->secondary_bank_capacity, sizeof(dst->secondary_bank_capacity));
		memcpy(dst->secondary_bank_rearm_time, src->secondary_bank_rearm_time, sizeof(dst->secondary_bank_rearm_time));
		memcpy(dst->secondary_bank_start_ammo, src->secondary_bank_start_ammo, sizeof(dst->secondary_bank_start_ammo));
		memcpy(dst->secondary_bank_weapons, src->secondary_bank_weapons, sizeof(dst->secondary_bank_weapons));
		memcpy(dst->secondary_next_slot, src->secondary_next_slot, sizeof(dst->secondary_next_slot));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_SECONDARY)));
}

ADE_VIRTVAR(TertiaryBanks, l_Ship, "weaponbanktype", "Array of tertiary weapon banks", "weaponbanktype", "Tertiary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_tertiary_bank = src->current_tertiary_bank;
		dst->num_tertiary_banks = src->num_tertiary_banks;

		dst->next_tertiary_fire_stamp = src->next_tertiary_fire_stamp;
		dst->tertiary_bank_ammo = src->tertiary_bank_ammo;
		dst->tertiary_bank_capacity = src->tertiary_bank_capacity;
		dst->tertiary_bank_rearm_time = src->tertiary_bank_rearm_time;
		dst->tertiary_bank_start_ammo = src->tertiary_bank_start_ammo;
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_TERTIARY)));
}

ADE_VIRTVAR(Target, l_Ship, "object", "Target of ship. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Target object, or invalid object handle if no target or ship handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	ai_info *aip = NULL;
	if(Ships[objh->objp->instance].ai_index > -1)
		aip = &Ai_info[Ships[objh->objp->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR && newh)
	{
		if(aip->target_signature != newh->sig)
		{
			if(newh->IsValid())
			{
				aip->target_objnum = OBJ_INDEX(newh->objp);
				aip->target_signature = newh->sig;
				aip->target_time = 0.0f;

				if (aip == Player_ai)
					hud_shield_hit_reset(newh->objp);
			}
			else
			{
				aip->target_objnum = -1;
				aip->target_signature = 0;
				aip->target_time = 0.0f;
			}

			set_targeted_subsys(aip, NULL, -1);
		}
	}

	return ade_set_object_with_breed(L, aip->target_objnum);
}

ADE_VIRTVAR(TargetSubsystem, l_Ship, "subsystem", "Target subsystem of ship.", "subsystem", "Target subsystem, or invalid subsystem handle if no target or ship handle is invalid")
{
	object_h *oh;
	ship_subsys_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ai_info *aip = NULL;
	if(Ships[oh->objp->instance].ai_index > -1)
		aip = &Ai_info[Ships[oh->objp->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if (aip == Player_ai) {
				if (aip->target_signature != newh->sig)
					hud_shield_hit_reset(newh->objp);

				Ships[newh->ss->parent_objnum].last_targeted_subobject[Player_num] = newh->ss;
			}

			aip->target_objnum = OBJ_INDEX(newh->objp);
			aip->target_signature = newh->sig;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, newh->ss, aip->target_objnum);
		}
		else
		{
			aip->target_objnum = -1;
			aip->target_signature = 0;
			aip->target_time = 0.0f;

			set_targeted_subsys(aip, NULL, -1);
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[aip->target_objnum], aip->targeted_subsys)));
}

ADE_VIRTVAR(Team, l_Ship, "team", "Ship's team", "team", "Ship team, or invalid team handle if ship handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	ship *shipp = &Ships[oh->objp->instance];

	if(ADE_SETTING_VAR && nt > -1) {
		shipp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(shipp->team));
}

ADE_VIRTVAR(Textures, l_Ship, "shiptextures", "Gets ship textures", "shiptextures", "Ship textures, or invalid shiptextures handle if ship handle is invalid")
{
	object_h *sh = nullptr;
	object_h *dh;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&dh), l_Ship.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ShipTextures.Set(object_h()));

	if(!dh->IsValid())
		return ade_set_error(L, "o", l_ShipTextures.Set(object_h()));

	if(ADE_SETTING_VAR && sh && sh->IsValid()) {
		ship *src = &Ships[sh->objp->instance];
		ship *dest = &Ships[dh->objp->instance];

		if (src->ship_replacement_textures != NULL)
		{
			if (dest->ship_replacement_textures == NULL)
				dest->ship_replacement_textures = (int *) vm_malloc(MAX_REPLACEMENT_TEXTURES * sizeof(int));

			memcpy(dest->ship_replacement_textures, src->ship_replacement_textures, MAX_REPLACEMENT_TEXTURES * sizeof(int));
		}
	}

	return ade_set_args(L, "o", l_ShipTextures.Set(object_h(dh->objp)));
}

ADE_VIRTVAR(FlagAffectedByGravity, l_Ship, "boolean", "Checks for the \"affected-by-gravity\" flag", "boolean", "True if flag is set, false if flag is not set and nil on error")
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Affected_by_gravity, set);
	}

	if (shipp->flags[Ship::Ship_Flags::Affected_by_gravity])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Disabled, l_Ship, "boolean", "The disabled state of this ship", "boolean", "true if ship is diabled, false otherwise")
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Disabled, set);
		if(set)
		{
			mission_log_add_entry(LOG_SHIP_DISABLED, shipp->ship_name, NULL );
		}
		else
		{
			ship_reset_disabled_physics( &Objects[shipp->objnum], shipp->ship_info_index );
		}
	}

	if (shipp->flags[Ship::Ship_Flags::Disabled])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Stealthed, l_Ship, "boolean", "Stealth status of this ship", "boolean", "true if stealthed, false otherwise or on error")
{
	object_h *objh=NULL;
	bool stealthed = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &stealthed))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Stealth, stealthed);
	}

	if (shipp->flags[Ship::Ship_Flags::Stealth])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HiddenFromSensors, l_Ship, "boolean", "Hidden from sensors status of this ship", "boolean", "true if invisible to hidden from sensors, false otherwise or on error")
{
	object_h *objh=NULL;
	bool hidden = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &hidden))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Hidden_from_sensors, hidden);
	}

	if (shipp->flags[Ship::Ship_Flags::Hidden_from_sensors])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Gliding, l_Ship, "boolean", "Specifies whether this ship is currently gliding or not.", "boolean", "true if gliding, false otherwise or in case of error")
{
	object_h *objh=NULL;
	bool gliding = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &gliding))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		if (Ship_info[shipp->ship_info_index].can_glide)
		{
			object_set_gliding(&Objects[shipp->objnum], gliding, true);
		}
	}

	if (objh->objp->phys_info.flags & PF_GLIDING || objh->objp->phys_info.flags & PF_FORCE_GLIDE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(EtsEngineIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Engine index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Engine Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].engine_recharge_index);
}

ADE_VIRTVAR(EtsShieldIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Shield index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Shield Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].shield_recharge_index);
}

ADE_VIRTVAR(EtsWeaponIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Weapon index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Weapon Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].weapon_recharge_index);
}

ADE_VIRTVAR(Orders, l_Ship, "shiporders", "Array of ship orders", "shiporders", "Ship orders, or invalid handle if ship handle is invalid")
{
	object_h *objh = NULL;
	object_h *newh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_ShipOrders.GetPtr(&newh)))
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));;

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Ai orders copy. Use giveOrder instead");
	}

	return ade_set_args(L, "o", l_ShipOrders.Set(object_h(objh->objp)));
}

ADE_FUNC(kill, l_Ship, "[object Killer]", "Kills the ship. Set \"Killer\" to the ship you are killing to self-destruct", "boolean", "True if successful, false or nil otherwise")
{
	object_h *victim,*killer=NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&victim), l_Ship.GetPtr(&killer)))
		return ADE_RETURN_NIL;

	if(!victim->IsValid())
		return ADE_RETURN_NIL;

	if(!killer || !killer->IsValid())
		return ADE_RETURN_NIL;

	//Ripped straight from shiphit.cpp
	float percent_killed = -get_hull_pct(victim->objp);
	if (percent_killed > 1.0f){
		percent_killed = 1.0f;
	}

	ship_hit_kill(victim->objp, killer->objp, percent_killed, (victim->sig == killer->sig) ? 1 : 0);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(addShipEffect, l_Ship, "string name, int duration (in milliseconds)", "Activates an effect for this ship. Effect names are defined in Post_processing.tbl, and need to be implemented in the main shader. This functions analogous to the ship-effect sexp. NOTE: only one effect can be active at any time, adding new effects will override effects already in progress.\n", "boolean", "Returns true if the effect was successfully added, false otherwise") {
	object_h *shiph;
	char* effect = NULL;
	int duration;
	int effect_num;

	if (!ade_get_args(L, "o|si", l_Ship.GetPtr(&shiph), &effect, &duration))
		return ade_set_error(L, "b", false);

	if (!shiph->IsValid())
		return ade_set_error(L, "b", false);

	effect_num = get_effect_from_name(effect);
	if (effect_num == -1)
		return ade_set_error(L, "b", false);

	ship* shipp = &Ships[shiph->objp->instance];

	shipp->shader_effect_active = true;
	shipp->shader_effect_num = effect_num;
	shipp->shader_effect_duration = duration;
	shipp->shader_effect_start_time = timer_get_milliseconds();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(hasShipExploded, l_Ship, NULL, "Checks if the ship explosion event has already happened", "number", "Returns 1 if first explosion timestamp is passed, 2 if second is passed, 0 otherwise")
{
	object_h *shiph;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "i", 0);

	if(!shiph->IsValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[shiph->objp->instance];

	if (shipp->flags[Ship::Ship_Flags::Dying]) {
		if (shipp->final_death_time == 0) {
			return ade_set_args(L, "i", 2);
		}
		if (shipp->pre_death_explosion_happened == 1) {
			return ade_set_args(L, "i", 1);
		}
		return ade_set_args(L, "i", 3);
	}

	return ade_set_args(L, "i", 0);
}

ADE_FUNC(fireCountermeasure, l_Ship, NULL, "Launches a countermeasure from the ship", "boolean", "Whether countermeasure was launched or not")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "b", false);

	if(!objh->IsValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", ship_launch_countermeasure(objh->objp));
}

ADE_FUNC(firePrimary, l_Ship, NULL, "Fires ship primary bank(s)", "number", "Number of primary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	int i = 0;
	i += ship_fire_primary(objh->objp, 0);
	i += ship_fire_primary(objh->objp, 1);

	return ade_set_args(L, "i", i);
}

ADE_FUNC(fireSecondary, l_Ship, NULL, "Fires ship secondary bank(s)", "number", "Number of secondary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_fire_secondary(objh->objp, 0));
}

ADE_FUNC(getAnimationDoneTime, l_Ship, "number Type, number Subtype", "Gets time that animation will be done", "number", "Time (seconds), or 0 if ship handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	int subtype=-1;
	if(!ade_get_args(L, "o|si", l_Ship.GetPtr(&objh), &s, &subtype))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	int type = model_anim_match_type(s);
	if(type < 0)
		return ADE_RETURN_FALSE;

	int time_ms = model_anim_get_time_type(&Ships[objh->objp->instance], type, subtype);
	float time_s = (float)time_ms / 1000.0f;

	return ade_set_args(L, "f", time_s);
}

ADE_FUNC(clearOrders, l_Ship, NULL, "Clears a ship's orders list", "boolean", "True if successful, otherwise false or nil")
{
	object_h *objh = NULL;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;
	if(!objh->IsValid())
		return ade_set_error(L, "b", false);

	//The actual clearing of the goals
	ai_clear_ship_goals( &Ai_info[Ships[objh->objp->instance].ai_index]);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(giveOrder, l_Ship, "enumeration Order, [object Target=nil, subsystem TargetSubsystem=nil, number Priority=1.0, shipclass TargetShipclass=nil]", "Uses the goal code to execute orders", "boolean", "True if order was given, otherwise false or nil")
{
	object_h *objh = NULL;
	enum_h *eh = NULL;
	float priority = 1.0f;
	int sclass = -1;
	object_h *tgh = NULL;
	ship_subsys_h *tgsh = NULL;
	if(!ade_get_args(L, "oo|oofo", l_Object.GetPtr(&objh), l_Enum.GetPtr(&eh), l_Object.GetPtr(&tgh), l_Subsystem.GetPtr(&tgsh), &priority, l_Shipclass.Get(&sclass)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid() || !eh->IsValid())
		return ade_set_error(L, "b", false);

	//wtf...
	if(priority < 0.0f)
		return ade_set_error(L, "b", false);

	if(priority > 1.0f)
		priority = 1.0f;

	bool tgh_valid = tgh && tgh->IsValid();
	bool tgsh_valid = tgsh && tgsh->IsValid();
	int ai_mode = AI_GOAL_NONE;
	int ai_submode = -1234567;
	char *ai_shipname = NULL;
	switch(eh->index)
	{
		case LE_ORDER_ATTACK:
		{
			if(tgsh_valid)
			{
				ai_mode = AI_GOAL_DESTROY_SUBSYSTEM;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = ship_get_subsys_index( &Ships[tgsh->objp->instance], tgsh->ss->system_info->subobj_name );
			}
			else if(tgh_valid && tgh->objp->type == OBJ_WEAPON)
			{
				ai_mode = AI_GOAL_CHASE_WEAPON;
				ai_submode = tgh->objp->instance;
			}
			else if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_CHASE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
		case LE_ORDER_DOCK:
		{
			ai_shipname = Ships[tgh->objp->instance].ship_name;
			ai_mode = AI_GOAL_DOCK;
			ai_submode = AIS_DOCK_0;
			break;
		}
		case LE_ORDER_WAYPOINTS:
		{
			if(tgh_valid && tgh->objp->type == OBJ_WAYPOINT)
			{
				ai_mode = AI_GOAL_WAYPOINTS;
				waypoint_list *wp_list = find_waypoint_list_with_instance(tgh->objp->instance);
				if(wp_list != NULL)
					ai_shipname = wp_list->get_name();
			}
			break;
		}
		case LE_ORDER_WAYPOINTS_ONCE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_WAYPOINT)
			{
				ai_mode = AI_GOAL_WAYPOINTS_ONCE;
				waypoint_list *wp_list = find_waypoint_list_with_instance(tgh->objp->instance);
				if(wp_list != NULL)
					ai_shipname = wp_list->get_name();
			}
			break;
		}
		case LE_ORDER_DEPART:
		{
			ai_mode = AI_GOAL_WARP;
			ai_submode = -1;
			break;
		}
		case LE_ORDER_FORM_ON_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FORM_ON_WING;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_UNDOCK:
		{
			ai_mode = AI_GOAL_UNDOCK;
			ai_submode = AIS_UNDOCK_0;

			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_GUARD:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_GUARD;
				ai_submode = AIS_GUARD_PATROL;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISABLE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_DISABLE_SHIP;
				ai_submode = -SUBSYSTEM_ENGINE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISARM:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_DISARM_SHIP;
				ai_submode = -SUBSYSTEM_TURRET;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_ATTACK_ANY:
		{
			ai_mode = AI_GOAL_CHASE_ANY;
			ai_submode = SM_ATTACK;
			break;
		}
		case LE_ORDER_IGNORE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_IGNORE_NEW;
				ai_submode = 0;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_EVADE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_EVADE_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_STAY_NEAR:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_STAY_NEAR_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_KEEP_SAFE_DISTANCE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_KEEP_SAFE_DISTANCE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_REARM:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_REARM_REPAIR;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_STAY_STILL:
		{
			ai_mode = AI_GOAL_STAY_STILL;
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_PLAY_DEAD:
		{
			ai_mode = AI_GOAL_PLAY_DEAD;
			break;
		}
		case LE_ORDER_FLY_TO:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FLY_TO_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_ATTACK_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_CHASE_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = SM_ATTACK;
				}
			}
			break;
		}
		case LE_ORDER_GUARD_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_GUARD_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = AIS_GUARD_STATIC;
				}
			}
			break;
		}
		case LE_ORDER_ATTACK_SHIP_CLASS:
		{
			if(sclass >= 0)
			{
				ai_mode = AI_GOAL_CHASE_SHIP_CLASS;
				ai_shipname = Ship_info[sclass].name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
	}

	//Nothing got set!
	if(ai_mode == AI_GOAL_NONE)
		return ade_set_error(L, "b", false);

	//Fire off the goal
	ai_add_ship_goal_scripting(ai_mode, ai_submode, (int)(priority*100.0f), ai_shipname, &Ai_info[Ships[objh->objp->instance].ai_index]);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(doManeuver, l_Ship, "number Duration, number Heading, number Pitch, number Bank, boolean Force Rotation, number Vertical, number Horizontal, number Forward, boolean Force Movement", "Sets ship maneuver over the defined time period", "boolean", "True if maneuver order was given, otherwise false or nil")
{
	object_h *objh;
	float arr[6];
	bool f_rot = false, f_move = false;
	int t, i;
	if(!ade_get_args(L, "oifffbfffb", l_Ship.GetPtr(&objh), &t, &arr[0], &arr[1], &arr[2], &f_rot, &arr[3], &arr[4], &arr[5], &f_move))
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];
	control_info *cip = &aip->ai_override_ci;

	aip->ai_override_timestamp = timestamp(t);
	aip->ai_override_flags.reset();

	if (t < 2)
		return ADE_RETURN_FALSE;

	for(i = 0; i < 6; i++) {
		if((arr[i] < -1.0f) || (arr[i] > 1.0f))
			arr[i] = 0;
	}

	if(f_rot) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full);
		cip->heading = arr[0];
		cip->pitch = arr[1];
		cip->bank = arr[2];
	} else {
		if (arr[0] != 0) {
			cip->heading = arr[0];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Heading);
		}
		if (arr[1] != 0) {
			cip->pitch = arr[1];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Pitch);
		}
		if (arr[2] != 0) {
			cip->bank = arr[2];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Roll);
		}
	}
	if(f_move) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full_lat);
		cip->vertical = arr[3];
		cip->sideways = arr[4];
		cip->forward = arr[5];
	} else {
		if (arr[3] != 0) {
			cip->vertical = arr[3];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Up);
		}
		if (arr[4] != 0) {
			cip->sideways = arr[4];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Sideways);
		}
		if (arr[5] != 0) {
			cip->forward = arr[5];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Forward);
		}
	}
	return ADE_RETURN_TRUE;
}

ADE_FUNC(triggerAnimation, l_Ship, "string Type, [number Subtype, boolean Forwards, boolean Instant]",
		 "Triggers an animation. Type is the string name of the animation type, "
			 "Subtype is the subtype number, such as weapon bank #, Forwards and Instant are boolean, defaulting to true & false respectively."
			 "<br><strong>IMPORTANT: Function is in testing and should not be used with official mod releases</strong>",
		 "boolean",
		 "True if successful, false or nil otherwise")
{
	object_h *objh;
	char *s = NULL;
	bool b = true;
	bool instant = false;
	int subtype=-1;
	if(!ade_get_args(L, "o|sibb", l_Ship.GetPtr(&objh), &s, &subtype, &b, &instant))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	int type = model_anim_match_type(s);
	if(type < 0)
		return ADE_RETURN_FALSE;

	int dir = 1;
	if(!b)
		dir = -1;

	model_anim_start_type(&Ships[objh->objp->instance], type, subtype, dir, instant);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(warpIn, l_Ship, NULL, "Warps ship in", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	shipfx_warpin_start(objh->objp);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(warpOut, l_Ship, NULL, "Warps ship out", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	shipfx_warpout_start(objh->objp);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(canWarp, l_Ship, NULL, "Checks whether ship has a working subspace drive and is allowed to use it", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	if(shipp->flags[Ship::Ship_Flags::No_subspace_drive]){
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

// Aardwolf's function for finding if a ship should be drawn as blue on the radar/minimap
ADE_FUNC(isWarpingIn, l_Ship, NULL, "Checks if ship is warping in", "boolean", "True if the ship is warping in, false or nil otherwise")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	if(shipp->flags[Ship::Ship_Flags::Arriving_stage_1]){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getEMP, l_Ship, NULL, "Returns the current emp effect strength acting on the object", "number", "Current EMP effect strength or NIL if object is invalid")
{
	object_h *objh = NULL;
	object *obj = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ADE_RETURN_NIL;
	}

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;

	ship *shipp = &Ships[obj->instance];

	return ade_set_args(L, "f", shipp->emp_intensity);
}

ADE_FUNC(getTimeUntilExplosion, l_Ship, NULL, "Returns the time in seconds until the ship explodes", "number", "Time until explosion or -1, if invalid handle or ship isn't exploding")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "f", -1.0f);
	}

	if(!objh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if (!timestamp_valid(shipp->final_death_time))
	{
		return ade_set_args(L, "f", -1.0f);
	}

	int time_until = timestamp_until(shipp->final_death_time);

	return ade_set_args(L, "f", (i2fl(time_until) / 1000.0f));
}

ADE_FUNC(getCallsign, l_Ship, NULL, "Gets the callsign of the ship in the current mission", "string", "The callsign or an empty string if the ship doesn't have a callsign or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if (shipp->callsign_index < 0)
		return ade_set_args(L, "s", "");

	char temp_callsign[NAME_LENGTH];

	*temp_callsign = 0;
	mission_parse_lookup_callsign_index(shipp->callsign_index, temp_callsign);

	if (*temp_callsign)
		return ade_set_args(L, "s", temp_callsign);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getAltClassName, l_Ship, NULL, "Gets the alternate class name of the ship", "string", "The alternate class name or an empty string if the ship doesn't have such a thing or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if (shipp->alt_type_index < 0)
		return ade_set_args(L, "s", "");

	char temp[NAME_LENGTH];

	*temp = 0;
	mission_parse_lookup_alt_index(shipp->alt_type_index, temp);

	if (*temp)
		return ade_set_args(L, "s", temp);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getMaximumSpeed, l_Ship, "[number energy = 0.333]", "Gets the maximum speed of the ship with the given energy on the engines", "number", "The maximum speed or -1 on error")
{
	object_h *objh = NULL;
	float energy = 0.333f;

	if (!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &energy)) {
		return ade_set_error(L, "f", -1.0f);
	}

	if(!objh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	if (energy < 0.0f || energy > 1.0f)
	{
		LuaError(L, "Invalid energy level %f! Needs to be in [0, 1].", energy);

		return ade_set_args(L, "f", -1.0f);
	}
	else
	{
		return ade_set_args(L, "f", ets_get_max_speed(objh->objp, energy));
	}
}

ADE_FUNC(EtsSetIndexes, l_Ship, "number Engine Index, number Shield Index, number Weapon Index",
		 "Sets ships ETS systems to specified values",
		 "boolean",
		 "True if successful, false if target ships ETS was missing, or only has one system")
{
	object_h *objh=NULL;
	int ets_idx[num_retail_ets_gauges] = {0};

	if (!ade_get_args(L, "oiii", l_Ship.GetPtr(&objh), &ets_idx[ENGINES], &ets_idx[SHIELDS], &ets_idx[WEAPONS]))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	sanity_check_ets_inputs(ets_idx);

	int sindex = objh->objp->instance;
	if (validate_ship_ets_indxes(sindex, ets_idx)) {
		Ships[sindex].engine_recharge_index = ets_idx[ENGINES];
		Ships[sindex].shield_recharge_index = ets_idx[SHIELDS];
		Ships[sindex].weapon_recharge_index = ets_idx[WEAPONS];
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(getWing, l_Ship, NULL, "Returns the ship's wing", "wing", "Wing handle, or invalid wing handle if ship is not part of a wing")
{
	object_h *objh = NULL;
	ship *shipp = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Wing.Set(-1));

	shipp = &Ships[objh->objp->instance];
	return ade_set_args(L, "o", l_Wing.Set(shipp->wingnum));
}


}
}

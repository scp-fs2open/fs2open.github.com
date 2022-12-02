#include "hook_conditions.h"

#include <functional>
#include <utility>

#include "gamesequence/gamesequence.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

// ---- Hook Condition System Macro and Class defines ----

#define HOOK_CONDITIONS_START(name) const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>> name::conditions = []() { \
	SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>> build;
#define HOOK_CONDITIONS_END return build; \
}();
#define HOOK_CONDITION(conditionsClassName, conditionParseName, documentation, argument, argumentParse, argumentValid) \
	build.emplace(conditionParseName, ::make_unique<ParseableConditionImpl<conditionsClassName, \
		decltype(std::declval<conditionsClassName>().argument), decltype(argumentParse())>> \
		(documentation, &conditionsClassName::argument, argumentParse, argumentValid))

namespace scripting {

template<typename conditions_t, typename operating_t, typename cache_t>
class ParseableConditionImpl : public ParseableCondition {
	const operating_t conditions_t::* object;
	std::function<cache_t(void)> cache;
	std::function<bool(operating_t, const cache_t&)> evaluate;

	template<typename conditions_t, typename operating_t, typename cache_t> friend class EvaluatableConditionImpl;
public:
	std::unique_ptr<EvaluatableCondition> parse() const override {
		return ::make_unique<EvaluatableConditionImpl<conditions_t, operating_t, cache_t>>(*this);
	}

	ParseableConditionImpl(SCP_string documentation_, const operating_t conditions_t::* object_, std::function<cache_t(void)> cache_, std::function<bool(operating_t, const cache_t&)> evaluate_) :
		ParseableCondition(std::move(documentation_)), object(object_), cache(std::move(cache_)), evaluate(std::move(evaluate_)) { }
};

template<typename conditions_t, typename operating_t, typename cache_t>
class EvaluatableConditionImpl : public EvaluatableCondition {
	const ParseableConditionImpl<conditions_t, operating_t, cache_t>& condition;
	cache_t cached;

public:
	EvaluatableConditionImpl(const ParseableConditionImpl<conditions_t, operating_t, cache_t>& _condition) : condition(_condition), cached(condition.cache()) { }

	bool evaluate(const linb::any& conditionContext) const override {
		const conditions_t& conditions = linb::any_cast<conditions_t>(conditionContext);
		return condition.evaluate(conditions.*(condition.object), cached);
	}
};


namespace hooks {

// ---- Hook Condition System Utility and Parsing methods ----

static bool conditionCompareShip(const ship* shipp, const SCP_string& value) {
	return shipp != nullptr && stricmp(shipp->ship_name, value.c_str()) == 0;
}

static bool conditionCompareShipType(const ship* shipp, const int& value) {
	return shipp != nullptr && Ship_info[shipp->ship_info_index].class_type == value;
}

static bool conditionCompareShipClass(const ship* shipp, const int& value) {
	return shipp != nullptr && shipp->ship_info_index == value;
}

static bool conditionCompareWeaponClass(const weapon* wep, const int& value) {
	return wep != nullptr && wep->weapon_info_index == value;
}

static bool conditionIsObjecttype(const object* objp, const int& value) {
	return objp != nullptr && objp->type == value;
}

template<typename fnc_t, typename value_t>
static bool conditionObjectIsShipDo(fnc_t fnc, const object* objp, const value_t& value) {
	if (objp != nullptr && objp->type == OBJ_SHIP) {
		return fnc(&Ships[objp->instance], value);
	}
	return false;
}

template<typename fnc_t, typename value_t>
static bool conditionObjectIsWeaponDo(fnc_t fnc, const object* objp, const value_t& value) {
	if (objp != nullptr && objp->type == OBJ_WEAPON) {
		return fnc(&Weapons[objp->instance], value);
	}
	return false;
}


static SCP_string conditionParseString() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return name;
}

static int conditionParseActionId() {
	SCP_string name;
	stuff_string(name, F_NAME);
	for (int i = 0; i < Control_config.size(); i++) {
		if (stricmp(Control_config[i].text.c_str(), name.c_str()) == 0)
			return i;
	}
	return -1;
}

static int conditionParseShipType() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return ship_type_name_lookup(name.c_str());
}

static int conditionParseShipClass() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return ship_info_lookup(name.c_str());
}

static int conditionParseWeaponClass() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return weapon_info_lookup(name.c_str());
}

static int conditionParseObjectType() {
	SCP_string name;
	stuff_string(name, F_NAME);
	for (int i = 0; i < MAX_OBJECT_TYPES; i++) {
		if (stricmp(Object_type_names[i], name.c_str()) == 0)
			return i;
	}
	return -1;
}

// ---- Hook Condition Helpers ----

#define HOOK_CONDITION_SHIPP(classname, documentationAddendum, shipp) \
	HOOK_CONDITION(classname, "Ship", "Specifies the name of the ship " documentationAddendum, shipp, conditionParseString, conditionCompareShip); \
	HOOK_CONDITION(classname, "Ship class", "Specifies the class of the ship " documentationAddendum, shipp, conditionParseShipClass, conditionCompareShipClass); \
	HOOK_CONDITION(classname, "Ship type", "Specifies the type of the ship " documentationAddendum, shipp, conditionParseShipType, conditionCompareShipType); 

#define HOOK_CONDITION_SHIP_OBJP(classname, documentationAddendum, objp) \
	HOOK_CONDITION(classname, "Ship", "Specifies the name of the ship " documentationAddendum, objp, conditionParseString, [](const object* objp, const SCP_string& shipname) -> bool { \
		return conditionObjectIsShipDo(&conditionCompareShip, objp, shipname); \
		}); \
	HOOK_CONDITION(classname, "Ship class", "Specifies the class of the ship " documentationAddendum, objp, conditionParseShipClass, [](const object* objp, const int& shipclass) -> bool { \
		return conditionObjectIsShipDo(&conditionCompareShipClass, objp, shipclass); \
		}); \
	HOOK_CONDITION(classname, "Ship type", "Specifies the type of the ship " documentationAddendum, objp, conditionParseShipType, [](const object* objp, const int& shiptype) -> bool { \
		return conditionObjectIsShipDo(&conditionCompareShipType, objp, shiptype); \
		});

// ---- Hook Conditions ----

HOOK_CONDITIONS_START(ControlActionConditions)
	HOOK_CONDITION(ControlActionConditions, "Action", "Specifies the action triggered by a keypress.", action_index, conditionParseActionId, [](int target, const int& key) -> bool {
		if (gameseq_get_depth() < 0)
			return false;
		return key == target;
	});
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ShipSourceConditions)
	HOOK_CONDITION_SHIPP(ShipSourceConditions, "that was the source of the event.", source_shipp);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(CollisionConditions)
	//These conditions with "must be one of the ships" instead of "must be both ships" is unlike the previous behaviour of PR #4231, but is closer to the original behaviour, where a specific ship of the two must match
	HOOK_CONDITION(CollisionConditions, "Ship", "Specifies the name of the ship which was part of the collision. At least one ship must be part of the collision and match.", participating_objects, conditionParseString, [](CollisionConditions::ParticipatingObjects po, const SCP_string& shipname) -> bool {
		if(conditionObjectIsShipDo(&conditionCompareShip, po.objp_a, shipname)) 
			return true;
		if (conditionObjectIsShipDo(&conditionCompareShip, po.objp_b, shipname))
			return true;
		return false;
	});
	HOOK_CONDITION(CollisionConditions, "Ship class", "Specifies the class of the ship which was part of the collision. At least one ship must be part of the collision and match.", participating_objects, conditionParseShipClass, [](CollisionConditions::ParticipatingObjects po, const int& shipclass) -> bool {
		if (conditionObjectIsShipDo(&conditionCompareShipClass, po.objp_a, shipclass))
			return true;
		if (conditionObjectIsShipDo(&conditionCompareShipClass, po.objp_b, shipclass))
			return true;
		return false;
	});
	HOOK_CONDITION(CollisionConditions, "Ship type", "Specifies the type of the ship which was part of the collision. At least one ship must be part of the collision and match.", participating_objects, conditionParseShipType, [](CollisionConditions::ParticipatingObjects po, const int& shiptype) -> bool {
		if (conditionObjectIsShipDo(&conditionCompareShipType, po.objp_a, shiptype))
			return true;
		if (conditionObjectIsShipDo(&conditionCompareShipType, po.objp_b, shiptype))
			return true;
		return false;
	});
	HOOK_CONDITION(CollisionConditions, "Weapon class", "Specifies the name of the weapon class which was part of the collision. At least one weapon must be part of the collision and match.", participating_objects, conditionParseWeaponClass, [](CollisionConditions::ParticipatingObjects po, const int& weaponclass) -> bool {
		if (conditionObjectIsWeaponDo(&conditionCompareWeaponClass, po.objp_a, weaponclass))
			return true;
		if (conditionObjectIsWeaponDo(&conditionCompareWeaponClass, po.objp_b, weaponclass))
			return true;
		return false;
	});
	HOOK_CONDITION(CollisionConditions, "Object type", "Specifies the type of the object which was part of the collision. At least one object must match.", participating_objects, conditionParseObjectType, [](CollisionConditions::ParticipatingObjects po, const int& objecttype) -> bool {
		if (conditionIsObjecttype(po.objp_a, objecttype))
			return true;
		if (conditionIsObjecttype(po.objp_b, objecttype))
			return true;
		return false;
	});
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ShipDeathConditions)
	HOOK_CONDITION_SHIPP(ShipDeathConditions, "that died.", dying_shipp);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(SubsystemDeathConditions)
	HOOK_CONDITION_SHIPP(SubsystemDeathConditions, "of which the subsystem got destroyed.", affected_shipp);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ShipDepartConditions)
	HOOK_CONDITION_SHIPP(ShipDepartConditions, "that departed.", leaving_shipp);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponDeathConditions)
	HOOK_CONDITION(WeaponDeathConditions, "Weapon class", "Specifies the class of the weapon that died.", dying_wep, conditionParseWeaponClass, conditionCompareWeaponClass);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ObjectDeathConditions)
	HOOK_CONDITION_SHIP_OBJP(ObjectDeathConditions, "that died.", dying_objp);
	HOOK_CONDITION(ObjectDeathConditions, "Weapon class", "Specifies the class of the weapon that died.", dying_objp, conditionParseWeaponClass, [](const object* objp, const int& weaponclass) -> bool {
		return conditionObjectIsWeaponDo(&conditionCompareWeaponClass, objp, weaponclass);
	});
	HOOK_CONDITION(ObjectDeathConditions, "Object type", "Specifies the type of the object that died.", dying_objp, conditionParseObjectType, conditionIsObjecttype);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ShipSpawnConditions)
	HOOK_CONDITION_SHIPP(ShipSpawnConditions, "that arrived.", spawned_shipp);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponCreatedConditions)
	HOOK_CONDITION_SHIP_OBJP(WeaponCreatedConditions, "that fired the weapon.", parent_objp);
	HOOK_CONDITION(WeaponCreatedConditions, "Object type", "Specifies the type of the object that is the parent of this weapon.", parent_objp, conditionParseObjectType, conditionIsObjecttype);
	HOOK_CONDITION(WeaponCreatedConditions, "Weapon class", "Specifies the class of the weapon that was fired.", spawned_wep, conditionParseWeaponClass, conditionCompareWeaponClass);
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponEquippedConditions)
	HOOK_CONDITION_SHIPP(WeaponEquippedConditions, "that has the weapon equipped.", user_shipp);
	HOOK_CONDITION(WeaponEquippedConditions, "Weapon class", "Specifies the class of the weapon that the ship needs to have equipped in at least one bank.", user_shipp, conditionParseWeaponClass, [](const ship* wielder, const int& weaponclass) -> bool {
		for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (wielder->weapons.primary_bank_weapons[i] > 0 && wielder->weapons.primary_bank_weapons[i] == weaponclass)
				return true;
		}
		for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (wielder->weapons.secondary_bank_weapons[i] > 0 && wielder->weapons.secondary_bank_weapons[i] == weaponclass)
				return true;
		}
		return false;
	});
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponUsedConditions)
	HOOK_CONDITION_SHIPP(WeaponUsedConditions, "that fired the weapon.", user_shipp);
	HOOK_CONDITION(WeaponUsedConditions, "Weapon class", "Specifies the class of the weapon that was fired.", weaponclass, conditionParseWeaponClass, &std::equal_to<int>());
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponSelectedConditions)
	HOOK_CONDITION_SHIPP(WeaponSelectedConditions, "that has selected the weapon.", user_shipp);
	HOOK_CONDITION(WeaponSelectedConditions, "Weapon class", "Specifies the class of the weapon that was selected.", weaponclass, conditionParseWeaponClass, &std::equal_to<int>());
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(WeaponDeselectedConditions)
	HOOK_CONDITION_SHIPP(WeaponDeselectedConditions, "that has selected the weapon.", user_shipp);
	HOOK_CONDITION(WeaponDeselectedConditions, "Weapon class", "Specifies the class of the weapon that was deselected.", weaponclass_prev, conditionParseWeaponClass, &std::equal_to<int>());
HOOK_CONDITIONS_END

HOOK_CONDITIONS_START(ObjectDrawConditions)
	HOOK_CONDITION_SHIP_OBJP(ObjectDrawConditions, "that was drawn / drawn from.", drawn_from_objp);
	HOOK_CONDITION(ObjectDrawConditions, "Weapon class", "Specifies the class of the weapon that was drawn / drawn from.", drawn_from_objp, conditionParseWeaponClass, [](const object* objp, const int& weaponclass) -> bool {
		return conditionObjectIsWeaponDo(&conditionCompareWeaponClass, objp, weaponclass);
		});
	HOOK_CONDITION(ObjectDrawConditions, "Object type", "Specifies the type of the object that was drawn / drawn from.", drawn_from_objp, conditionParseObjectType, conditionIsObjecttype);
HOOK_CONDITIONS_END

}
}
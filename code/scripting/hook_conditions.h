#pragma once

#include <linb/any.hpp>

class object;
class ship;
struct weapon;
class ship_subsys;

#define HOOK_DEFINE_CONDITIONS static const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>> conditions

namespace scripting {

class EvaluatableCondition {
public:
	virtual bool evaluate(const linb::any& /*conditionContext*/) const {
		return false;
	};

	virtual ~EvaluatableCondition() = default;
};

class ParseableCondition {
public:
	SCP_string documentation;

	virtual std::unique_ptr<EvaluatableCondition> parse(const SCP_string& /*input*/) const {
		return make_unique<EvaluatableCondition>();
	};

	ParseableCondition() : documentation("Invalid Condition. Will never evaluate.") { }

	virtual ~ParseableCondition() = default;
protected:
	ParseableCondition(SCP_string documentation_) : documentation(std::move(documentation_)) { }
};

namespace hooks {

// ---- Hook Condition System Conditions ----

struct ControlActionConditions {
	HOOK_DEFINE_CONDITIONS;
	int action_index;
};

struct ShipSourceConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* source_shipp;
};

struct CollisionConditions {
	HOOK_DEFINE_CONDITIONS;
	struct ParticipatingObjects {
		const object* objp_a, * objp_b;
	} participating_objects;
};

struct ShipDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* dying_shipp;
};

struct SubsystemDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* affected_shipp;
	const ship_subsys* destroyed_subsys; // As of yet unused
};

struct ShipDepartConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* leaving_shipp;
};

struct WeaponDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const weapon* dying_wep;
};

struct ObjectDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const object* dying_objp;
};

struct ShipArriveConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* spawned_shipp;
	int arrival_location; // As of yet unused
	const object* spawn_anchor_objp; // As of yet unused
};

struct WeaponCreatedConditions {
	HOOK_DEFINE_CONDITIONS;
	const weapon* spawned_wep;
	const object* parent_objp;
};

struct WeaponEquippedConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* user_shipp;
	const object* target_objp; // As of yet unused
};

struct WeaponUsedConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* user_shipp;
	const object* target; // As of yet unused
	int weaponclass;
	bool isPrimary; // As of yet unused
};

struct WeaponSelectedConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* user_shipp;
	int weaponclass, weaponclass_prev;
	bool isPrimary; // As of yet unused
};

struct WeaponDeselectedConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* user_shipp;
	int weaponclass, weaponclass_prev;
	bool isPrimary; // As of yet unused
};

struct ObjectDrawConditions {
	HOOK_DEFINE_CONDITIONS;
	const object* drawn_from_objp;
};

}
}

#undef HOOK_DEFINE_CONDITIONS

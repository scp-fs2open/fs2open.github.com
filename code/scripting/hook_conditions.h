#pragma once

#include <linb/any.hpp>

class object;
class ship;
struct weapon;

#define HOOK_DEFINE_CONDITIONS static const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>> conditions

namespace scripting {

class EvaluatableCondition {
public:
	virtual bool evaluate(const linb::any& conditionContext) const {
		return false;
	};
};

class ParseableCondition {
public:
	SCP_string documentation;

	virtual std::unique_ptr<EvaluatableCondition> parse() const {
		return make_unique<EvaluatableCondition>();
	};

	ParseableCondition() : documentation("Invalid Condition. Will never evaluate.") { }
protected:
	ParseableCondition(SCP_string documentation_) : documentation(std::move(documentation_)) { }
};

namespace hooks {

// ---- Hook Condition System Conditions ----

struct ShipDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* dying_shipp;
};

struct WeaponDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const weapon* dying_wep;
};

struct ObjectDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const object* dying_objp;
};

struct ShipSourceConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* source_shipp;
};

struct DebrisCreatedConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* debris_shipp;
};

struct CollisionConditions {
	HOOK_DEFINE_CONDITIONS;
	struct ParticipatingObjects{
		const object *objp_a, *objp_b;
	} participating_objects;
};

}
}

#undef HOOK_DEFINE_CONDITIONS

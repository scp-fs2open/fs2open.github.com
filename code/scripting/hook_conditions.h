#pragma once

#include <linb/any.hpp>

#include "ship/ship.h"

class EvaluatableCondition {
public:
	virtual bool evaluate(linb::any conditionContext) const {
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

#define HOOK_DEFINE_CONDITIONS static const SCP_unordered_map<SCP_string, std::unique_ptr<ParseableCondition>> conditions


// ---- Hook Condition System Conditions ----

struct ShipDeathConditions {
	HOOK_DEFINE_CONDITIONS;
	const ship* dying_shipp;
};
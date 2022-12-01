#include "hook_conditions.h"

#include <utility>

#include "ship/ship.h"

// ---- Hook Condition System Macro and Class defines ----

#define HOOK_CONDITIONS_START(name) const SCP_unordered_map<SCP_string, std::unique_ptr<ParseableCondition>> name::conditions = []() { \
	SCP_unordered_map<SCP_string, std::unique_ptr<ParseableCondition>> build;
#define HOOK_CONDITIONS_END return build; \
}();
#define HOOK_CONDITION(conditionsClassName, conditionParseName, documentation, argument, argumentParse, argumentValid) \
	build.emplace(conditionParseName, ::make_unique<ParseableConditionImpl<conditionsClassName, \
		decltype(std::declval<conditionsClassName>().argument), decltype(argumentParse())>> \
		(documentation, &conditionsClassName::argument, argumentParse, argumentValid))

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

	bool evaluate(linb::any conditionContext) const override {
		const conditions_t conditions = linb::any_cast<conditions_t>(std::move(conditionContext));
		return condition.evaluate(conditions.*(condition.object), cached);
	}
};


// ---- Hook Condition System Utility and Parsing methods ----

bool conditionCompareShip(const ship* shipp, const SCP_string& value) {
	return stricmp(shipp->ship_name, value.c_str()) == 0;
}

bool conditionCompareShipType(const ship* shipp, const int& value) {
	return Ship_info[shipp->ship_info_index].class_type == value;
}

bool conditionCompareShipClass(const ship* shipp, const int& value) {
	return shipp->ship_info_index == value;
}


SCP_string conditionParseString() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return name;
}

int conditionParseShipType() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return ship_type_name_lookup(name.c_str());
}

int conditionParseShipClass() {
	SCP_string name;
	stuff_string(name, F_NAME);
	return ship_info_lookup(name.c_str());
}


// ---- Hook Condition Helpers ----

#define HOOK_CONDITION_SHIPP(classname, documentationAddendum, shipp) \
	HOOK_CONDITION(classname, "Ship", "Specifies the name of the ship " documentationAddendum, shipp, conditionParseString, conditionCompareShip); \
	HOOK_CONDITION(classname, "Ship class", "Specifies the class of the ship " documentationAddendum, shipp, conditionParseShipClass, conditionCompareShipClass); \
	HOOK_CONDITION(classname, "Ship type", "Specifies the type of the ship " documentationAddendum, shipp, conditionParseShipType, conditionCompareShipType); 


// ---- Hook Conditions ----

HOOK_CONDITIONS_START(ShipDeathConditions)
	HOOK_CONDITION_SHIPP(ShipDeathConditions, "that died", dying_shipp);
HOOK_CONDITIONS_END
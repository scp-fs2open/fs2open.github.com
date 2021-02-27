
#include "ActionDefinitionManager.h"

#include "BuiltinActionDefinition.h"

#include "actions/types/MoveToSubmodel.h"
#include "actions/types/ParticleEffectAction.h"
#include "actions/types/PlaySoundAction.h"
#include "actions/types/SetDirectionAction.h"
#include "actions/types/SetPositionAction.h"
#include "actions/types/WaitAction.h"
#include "utils/join_string.h"

namespace actions {
namespace {

void parseActionParameters(const SCP_vector<ActionParameter>& params,
	SCP_unordered_map<SCP_string, expression::ActionExpression>& parameterExpressions,
	const expression::ParseContext& context)
{
	// We do not impose a set order of the parameters so we need to do something similar to the outer loop and
	// check for every possibility in a loop.
	for (size_t i = 0; i < params.size(); ++i) {
		// We might be doing some empty loops here in case of errors but there is a bound so this will finish
		// eventually
		for (const auto& parameter : params) {
			// Do not check for parameters which we already found
			if (parameterExpressions.find(parameter.name) != parameterExpressions.end()) {
				continue;
			}

			const auto paramString = "+" + parameter.name + ":";

			if (!optional_string(paramString.c_str())) {
				continue;
			}

			// We found our parameter!
			parameterExpressions[parameter.name] =
				expression::ActionExpression::parseFromTable(parameter.type, context);
			break;
		}
	}
}

} // namespace

const ActionDefinitionManager& ActionDefinitionManager::instance()
{
	static const ActionDefinitionManager manager;
	return manager;
}
void ActionDefinitionManager::addDefinition(std::unique_ptr<ActionDefinition> def)
{
	m_definitions.push_back(std::move(def));
}
std::unique_ptr<Action> ActionDefinitionManager::parseAction(const flagset<ProgramContextFlags>& context_flags,
	const expression::ParseContext& context) const
{
	// We search through all definitions for every time we want to check for an action. This is not the most efficient
	// way to handle this since it results in an O(n^2) runtime. A better alternative would be to either construct a
	// regex with all the accepted actions as alternatives (although runtime of this is also doubtful) or peek at the
	// next token, extract the relevant part (The stuff between "+" and ":") and then look that up in a map. Then we
	// would get down to O(n).
	// Since this only happens at parse time, this is good enough for now.
	for (const auto& actionDef : m_definitions) {
		const auto parseString = "+" + actionDef->getName() + ":";

		if (!optional_string(parseString.c_str())) {
			continue;
		}

		SCP_unordered_map<SCP_string, expression::ActionExpression> parameterExpressions;
		const auto& params = actionDef->getParameters();

		if (params.size() == 1) {
			// Single parameters are expected to be placed directly after the action name
			parameterExpressions[params.front().name] =
				expression::ActionExpression::parseFromTable(params.front().type, context);
		} else {
			parseActionParameters(params, parameterExpressions, context);

			if (parameterExpressions.size() != params.size()) {
				SCP_vector<ActionParameter> missingParameters;
				std::copy_if(params.cbegin(),
					params.cend(),
					std::back_inserter(missingParameters),
					[&parameterExpressions](const ActionParameter& param) {
						const auto iter = parameterExpressions.find(param.name);
						return iter == parameterExpressions.cend();
					});

				SCP_vector<SCP_string> missingParameterNames;
				std::transform(missingParameters.cbegin(),
					missingParameters.cend(),
					std::back_inserter(missingParameterNames),
					[](const ActionParameter& param) { return "+" + param.name; });

				const auto namesList = util::join_container(missingParameterNames, ", ");

				error_display(0, "Missing parameters for action '%s': %s", parseString.c_str(), namesList.c_str());

				// Fix up the parameter list so that we can continue parsing
				for (const auto& missingParam : missingParameters) {
					parameterExpressions[missingParam.name] = expression::ActionExpression();
				}
			}
		}

		auto actionInstance = actionDef->createAction(parameterExpressions);

		const auto requiredFlags = actionDef->getRequiredContext();

		if ((context_flags & requiredFlags) != requiredFlags) {
			// We are missing at least one context flag so this action is not valid here. We still return the action
			// here though since that is the only way we can keep parsing the table without throwing random error
			// messages at the user. However, this will trigger an Assertion (in debug) or crashes when the action is
			// actually run later.
			error_display(0,
				"The action %s is not available in the current context. This will lead to an Assertion failure when "
				"this action is triggered.",
				parseString.c_str());
		}

		return actionInstance;
	}

	return std::unique_ptr<Action>();
}
ActionDefinitionManager::ActionDefinitionManager()
{
	addDefinition(
		std::unique_ptr<ActionDefinition>(new BuiltinActionDefinition<types::MoveToSubmodel>("Move To Subobject")));
	addDefinition(std::unique_ptr<ActionDefinition>(
		new BuiltinActionDefinition<types::ParticleEffectAction>("Start Particle Effect")));
	addDefinition(
		std::unique_ptr<ActionDefinition>(new BuiltinActionDefinition<types::PlaySoundAction>("Play 3D Sound")));
	addDefinition(
		std::unique_ptr<ActionDefinition>(new BuiltinActionDefinition<types::SetDirectionAction>("Set Direction")));
	addDefinition(
		std::unique_ptr<ActionDefinition>(new BuiltinActionDefinition<types::SetPositionAction>("Set Position")));
	addDefinition(std::unique_ptr<ActionDefinition>(new BuiltinActionDefinition<types::WaitAction>("Wait")));
}

} // namespace actions

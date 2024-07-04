#pragma once

#include "ActionDefinition.h"

namespace actions {

class ActionDefinitionManager {
  public:
	// This type is supposed to be global so it may not be copied
	ActionDefinitionManager(const ActionDefinitionManager&) = delete;
	ActionDefinitionManager& operator=(const ActionDefinitionManager&) = delete;

	ActionDefinitionManager(ActionDefinitionManager&&) noexcept = default;
	ActionDefinitionManager& operator=(ActionDefinitionManager&&) noexcept = default;

	std::unique_ptr<Action> parseAction(const flagset<ProgramContextFlags>& context_flags,
		const expression::ParseContext& context) const;

	static const ActionDefinitionManager& instance();

  private:
	ActionDefinitionManager();

	void addDefinition(std::unique_ptr<ActionDefinition> def);

	SCP_vector<std::unique_ptr<ActionDefinition>> m_definitions;
};

} // namespace actions

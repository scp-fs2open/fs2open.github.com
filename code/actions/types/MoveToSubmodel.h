#pragma once

#include "actions/Action.h"
#include "particle/ParticleManager.h"

namespace actions {
namespace types {

class MoveToSubmodel : public Action {
	bool _goToParent;
	SCP_string _destination;

  public:
	~MoveToSubmodel() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions

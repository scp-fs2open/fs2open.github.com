#pragma once

#include "actions/Action.h"
#include "particle/ParticleManager.h"

namespace actions {
namespace types {

class ParticleEffectAction : public Action {
	particle::ParticleEffectHandle _effectHandle;

  public:
	~ParticleEffectAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions

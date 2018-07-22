//
//

#include "WaitAction.h"

#include "io/timer.h"
#include "parse/parselo.h"

namespace actions {
namespace types {

WaitAction::~WaitAction() = default;
ActionResult WaitAction::execute(actions::ProgramLocals& locals) const
{
	if (!timestamp_valid(locals.waitTimestamp)) {
		locals.waitTimestamp = timestamp(fl2i(_waitTime.next()) * TIMESTAMP_FREQUENCY);
	}
	if (!timestamp_elapsed(locals.waitTimestamp)) {
		// Wait until the timestamp is elapsed
		return ActionResult::Wait;
	}
	// Timestamp is elapsed. The timestamp is elapsed so that timestamp_valid above returns false for the next wait
	// action
	locals.waitTimestamp = 0;
	return ActionResult::Finished;
}
void WaitAction::parseValues(const flagset<ProgramContextFlags>&)
{
	_waitTime = util::parseUniformRange(0.001f);
}
std::unique_ptr<Action> WaitAction::clone() const
{
	return std::unique_ptr<Action>(new WaitAction(*this));
}

} // namespace types
} // namespace actions

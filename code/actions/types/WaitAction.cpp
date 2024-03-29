
#include "WaitAction.h"

#include "io/timer.h"
#include "parse/parselo.h"

namespace actions {
namespace types {

flagset<ProgramContextFlags> WaitAction::getRequiredExecutionContextFlags()
{
	return flagset<ProgramContextFlags>{};
}

WaitAction::WaitAction(expression::TypedActionExpression<float> waitTimeExpression)
	: _waitTimeExpression(std::move(waitTimeExpression))
{
}
WaitAction::~WaitAction() = default;

ActionResult WaitAction::execute(actions::ProgramLocals& locals) const
{
	if (!locals.waitTimestamp.isValid()) {
		auto waitTime = _waitTimeExpression.execute(locals.variables);

		// Catch errors in the expression
		if (waitTime < 0.001f) {
			waitTime = 0.001f;
		}

		locals.waitTimestamp = _timestamp(fl2i(waitTime * MILLISECONDS_PER_SECOND));
	}
	if (!timestamp_elapsed(locals.waitTimestamp)) {
		// Wait until the timestamp is elapsed
		return ActionResult::Wait;
	}
	// Timestamp is elapsed. The timestamp is elapsed so that timestamp_valid above returns false for the next wait
	// action
	locals.waitTimestamp = TIMESTAMP::invalid();
	return ActionResult::Finished;
}
std::unique_ptr<Action> WaitAction::clone() const
{
	return std::unique_ptr<Action>(new WaitAction(*this));
}

} // namespace types
} // namespace actions

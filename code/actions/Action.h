#pragma once

#include "common.h"

namespace actions {

enum class ActionResult
{
	Finished,
	Errored,
	Wait
};

class Action {
  public:
	virtual ~Action() = 0;

	virtual ActionResult execute(ProgramLocals& locals) const = 0;

	virtual std::unique_ptr<Action> clone() const = 0;
};

} // namespace actions

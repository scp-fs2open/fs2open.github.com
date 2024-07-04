#pragma once

#include "IExecutionContext.h"

#include "gamesequence/gamesequence.h"
#include "utils/event.h"

namespace executor {

class CombinedExecutionContext : public IExecutionContext {
  public:
	CombinedExecutionContext(SCP_vector<std::shared_ptr<IExecutionContext>> contexts);
	~CombinedExecutionContext() override;

	State determineContextState() const override;

  private:
	SCP_vector<std::shared_ptr<IExecutionContext>> m_contexts;
};

} // namespace executor

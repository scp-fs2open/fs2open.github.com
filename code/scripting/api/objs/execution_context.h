
#pragma once

#include "executor/IExecutionContext.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class execution_context_h {
  public:
	execution_context_h(std::shared_ptr<executor::IExecutionContext> executionContext);

	const std::shared_ptr<executor::IExecutionContext>& getExecutionContext() const;

	bool isValid() const;

  private:
	std::shared_ptr<executor::IExecutionContext> m_executionContext;
};

DECLARE_ADE_OBJ(l_ExecutionContext, execution_context_h);

} // namespace api
} // namespace scripting

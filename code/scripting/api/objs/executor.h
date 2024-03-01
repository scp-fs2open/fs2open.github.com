
#pragma once

#include "executor/Executor.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class executor_h {
  public:
	executor_h();
	executor_h(std::shared_ptr<executor::Executor> executor);

	bool isValid() const;

	const std::shared_ptr<executor::Executor>& getExecutor() const;

  private:
	std::shared_ptr<executor::Executor> m_executor;
};

DECLARE_ADE_OBJ(l_Executor, executor_h);

} // namespace api
} // namespace scripting


#include "global_executors.h"

namespace executor {

const std::shared_ptr<Executor> OnSimulationExecutor = std::make_shared<Executor>();

const std::shared_ptr<Executor> OnFrameExecutor = std::make_shared<Executor>();

} // namespace executor

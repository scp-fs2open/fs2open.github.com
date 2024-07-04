#pragma once

#include "Executor.h"

namespace executor
{

/**
 * @brief Executor that will be called at the end of every simulation frame.
 */
extern const std::shared_ptr<Executor> OnSimulationExecutor;

/**
 * @brief Executor that will be called just before the game triggers the presentation of the current frame.
 */
extern const std::shared_ptr<Executor> OnFrameExecutor;

}

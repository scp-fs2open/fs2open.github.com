#pragma once

#include "globalincs/pstypes.h"

#include <mutex>

namespace executor {

/**
 * @brief A class that collects work that should be executed repeatedly.
 *
 * The intention behind this is that the engine code creates executor objects and then calls process() on them at
 * specific points of the engine code. External code can then post() work items to be executed when that point of the
 * code is reached without introducing a hard dependency between the two code modules.
 *
 * Work items have the option of specifying that they should be rescheduled for the next execution round.
 *
 * @note This class is thread safe and work items can be posted to the executor from different threads without risking
 * data corruption.
 */
class Executor {
  public:
	enum class CallbackResult {
		Done,       //! The work item is finished
		Reschedule, //! The work item should be executed again in the next round
	};

	using Callback = std::function<CallbackResult()>;

	/**
	 * @brief Adds a work item to this executor
	 *
	 * This work item will be executed every time process() is called until it returns CallbackResult::Done.
	 *
	 * @param cb The work item
	 */
	void post(Callback cb);

	/**
	 * @brief Executes one round of work items
	 */
	void process();

  private:
	std::mutex m_mainMutex;
	SCP_vector<Callback> m_workItems;

	std::mutex m_pendingWorkItemsMutex;
	SCP_vector<Callback> m_pendingWorkItems;
};

Executor* currentExecutor();

} // namespace executor


#include "Executor.h"

#include "utils/finally.h"

namespace executor {

namespace {
thread_local Executor* currentExecutorRef = nullptr;
}

Executor* currentExecutor() { return currentExecutorRef; }

void Executor::post(Executor::Callback cb)
{
	// To avoid deadlocks or recusive mutexes we have a temporary list which contains work items not added to the queue
	// yet. Those items will be transferred to the main queue in the next process call.
	std::unique_lock<std::mutex> lock(m_pendingWorkItemsMutex);
	m_pendingWorkItems.push_back(std::move(cb));
}

void Executor::process()
{
	// Ensure that no deadlocks happen when this is called from multiple threads
	std::lock(m_mainMutex, m_pendingWorkItemsMutex);

	Assertion(currentExecutorRef == nullptr, "Executor is already processing! Only one is allowed at a time.");
	currentExecutorRef = this;
	// Clean up ref after this function
	auto _ = util::finally([]() { currentExecutorRef = nullptr; });

	std::lock_guard<std::mutex> lk1(m_mainMutex, std::adopt_lock);
	{
		// Only need this for a limited time so we do this in a separate scope
		std::lock_guard<std::mutex> lk2(m_pendingWorkItemsMutex, std::adopt_lock);

		// Now move the pending items over to the actual work list
		std::move(m_pendingWorkItems.begin(), m_pendingWorkItems.end(), std::back_inserter(m_workItems));
		m_pendingWorkItems.clear();
	}

	for (auto iter = m_workItems.begin(); iter != m_workItems.end();) {
		auto& cb = *iter;
		if (cb() == CallbackResult::Done) {
			// This callback does not want to be called again
			// if we're sitting on the very last work item, popping-back will invalidate the iterator!
			if (iter + 1 == m_workItems.end()) {
				m_workItems.pop_back();
				break;
			}

			// Remove entry without copying it since the current iterator value is not needed anymore
			std::iter_swap(iter, std::prev(m_workItems.end()));
			m_workItems.pop_back();
			continue;
		}

		// next particle
		++iter;
	}
}

} // namespace executor

#pragma once

#include <cstdint>

namespace threading {
	enum class WorkerThreadTask : uint8_t { EXIT, COLLISION };

	//Call this to start a task on the task pool. Note that task-specific data must be set up before calling this.
	void spin_up_threaded_task(WorkerThreadTask task);

	//This _must_ be called on the main thread BEFORE a task completes on a thread of the task pool.
	void spin_down_threaded_task();

	void init_task_pool();
	void shut_down_task_pool();

	bool is_threading();
	size_t get_num_workers();
}
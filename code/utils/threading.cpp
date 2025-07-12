#include "threading.h"

#include "cmdline/cmdline.h"
#include "object/objcollide.h"
#include "globalincs/pstypes.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace threading {
	std::condition_variable wait_for_task;
	std::mutex wait_for_task_mutex;
	bool wait_for_task_condition;
	std::atomic<WorkerThreadTask> worker_task;

	SCP_vector<std::thread> worker_threads;

	//Internal Functions
	static void mp_worker_thread_main(size_t threadIdx) {
		while(true) {
			{
				std::unique_lock<std::mutex> lk(wait_for_task_mutex);
				wait_for_task.wait(lk, []() { return wait_for_task_condition; });
			}

			switch (worker_task.load(std::memory_order_acquire)) {
				case WorkerThreadTask::EXIT:
					return;
				case WorkerThreadTask::COLLISION:
					collide_mp_worker_thread(threadIdx);
					break;
				default:
					UNREACHABLE("Invalid threaded worker task!");
			}
		}
	}

	//External Functions

	void spin_up_threaded_task(WorkerThreadTask task) {
		worker_task.store(task);
		{
			std::scoped_lock lock {wait_for_task_mutex};
			wait_for_task_condition = true;
			wait_for_task.notify_all();
		}
	}

	void spin_down_threaded_task() {
		std::scoped_lock lock {wait_for_task_mutex};
		wait_for_task_condition = false;
	}

	void init_task_pool() {
		if (!is_threading())
			return;

		mprintf(("Spinning up threadpool with %d threads...\n", Cmdline_multithreading - 1));

		for (size_t i = 0; i < static_cast<size_t>(Cmdline_multithreading - 1); i++) {
			worker_threads.emplace_back([i](){ mp_worker_thread_main(i); });
		}
	}

	void shut_down_task_pool() {
		spin_up_threaded_task(WorkerThreadTask::EXIT);

		for(auto& thread : worker_threads) {
			thread.join();
		}
	}

	bool is_threading() {
		return Cmdline_multithreading > 1;
	}

	size_t get_num_workers() {
		return worker_threads.size();
	}
}
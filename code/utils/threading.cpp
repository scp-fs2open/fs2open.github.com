#include "threading.h"

#include "cmdline/cmdline.h"
#include "object/objcollide.h"
#include "globalincs/pstypes.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#ifdef WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#endif

namespace threading {
	static size_t num_threads = 1;
	static std::condition_variable wait_for_task;
	static std::mutex wait_for_task_mutex;
	static bool wait_for_task_condition;
	static std::atomic<WorkerThreadTask> worker_task;

	static SCP_vector<std::thread> worker_threads;

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

	static size_t get_number_of_physical_cores_fallback() {
		unsigned int hardware_threads = std::thread::hardware_concurrency();
		if (hardware_threads > 0) {
			return hardware_threads;
		}
		else {
			Warning(LOCATION, "Could not autodetect available number of threads! Disabling multithreading...");
			return 1;
		}
	}

	//We don't want to rely on std::thread::hardware_concurrency() unless we have to, as it reports threads, not physical cores, and FSO doesn't gain much from hyperthreaded threads at the moment.
#ifdef WIN32
	static size_t get_number_of_physical_cores() {
		auto glpi = (BOOL (WINAPI *)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD)) GetProcAddress(
				GetModuleHandle(TEXT("kernel32")),
				"GetLogicalProcessorInformation");

		if (glpi == nullptr)
			return get_number_of_physical_cores_fallback();

		DWORD length = 0;
		glpi(nullptr, &length);
		SCP_vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> infoBuffer(length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
		DWORD error = glpi(infoBuffer.data(), &length);

		if (error == 0)
			return get_number_of_physical_cores_fallback();

		size_t num_cores = 0;
		for (const auto& info : infoBuffer) {
			if (info.Relationship == RelationProcessorCore && info.ProcessorMask != 0)
				num_cores++;
		}

		if (num_cores < 1) {
			//invalid results, try fallback
			return get_number_of_physical_cores_fallback();
		}
		else {
			return num_cores;
		}
	}
#elif defined __APPLE__
	static size_t get_number_of_physical_cores() {
		int rval = 0;
		int num = 0;
		size_t numSize = sizeof(num);

		// apple silicon (performance cores only)
		rval = sysctlbyname("hw.perflevel0.physicalcpu", &num, &numSize, nullptr, 0);

		// intel
		if (rval != 0) {
			rval = sysctlbyname("hw.physicalcpu", &num, &numSize, nullptr, 0);
		}

		if (rval == 0 && num > 0) {
			return num;
		} else {
			// invalid results, try fallback
			return get_number_of_physical_cores_fallback();
		}
	}
#elif defined SCP_UNIX
	static size_t get_number_of_physical_cores() {
		try {
			std::ifstream cpuinfo("/proc/cpuinfo");
			SCP_string line;
			while (std::getline(cpuinfo, line)) {
				//Looking for a cpu cores property is fine assuming a user has only one physical CPU socket. If they have multiple CPU's, this'll underreport the core count, but that should be very rare in typical configurations
				if (line.find("cpu cores") != SCP_string::npos){
					size_t numberpos = line.find(": ");
					if (numberpos == SCP_string::npos)
						return get_number_of_physical_cores_fallback();

					int num_cores = std::stoi(line.substr(numberpos + 2));

					if (num_cores < 1) {
						//invalid results, try fallback
						return get_number_of_physical_cores_fallback();
					}
					else {
						return num_cores;
					}
				}
			}
			return get_number_of_physical_cores_fallback();
		}
		catch (const std::exception&) {
			return get_number_of_physical_cores_fallback();
		}
	}
#else
#define get_number_of_physical_cores() get_number_of_physical_cores_fallback()
#endif

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
		if (Cmdline_multithreading == 0) {
			//At least given the current collision-detection threading, 8 cores (if available) seems like a sweetspot, with more cores adding too much overhead.
			//This could be improved in the future.
			//This could also be made task-dependant, if stuff like parallelized loading benefits from more cores.
			num_threads = std::min(get_number_of_physical_cores() - 1, static_cast<size_t>(7));
		}
		else {
			num_threads = Cmdline_multithreading - 1;
		}

		if (!is_threading())
			return;

		mprintf(("Spinning up threadpool with %d threads...\n", static_cast<int>(num_threads)));

		for (size_t i = 0; i < num_threads; i++) {
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
		return num_threads > 0;
	}

	size_t get_num_workers() {
		return worker_threads.size();
	}
}

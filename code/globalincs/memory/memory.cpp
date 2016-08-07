#include "globalincs/memory/memory.h"

#ifndef NDEBUG

#include "globalincs/memory/debug.h"

#endif

#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>

#ifdef new
#undef new
#endif

namespace
{
	// We need to use an allocator that uses std::malloc specifically so we don't cause infinite recursion
	template<class T>
	struct malloc_allocator
	{
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T *pointer;
		typedef const T *const_pointer;
		typedef T &reference;
		typedef const T &const_reference;
		typedef T value_type;

		template<class U>
		struct rebind
		{
			typedef malloc_allocator<U> other;
		};

		malloc_allocator() SCP_NOEXCEPT
		{ }

		malloc_allocator(const malloc_allocator &) SCP_NOEXCEPT
		{ }

		template<class U>
		malloc_allocator(const malloc_allocator<U> &) SCP_NOEXCEPT
		{ }

		~malloc_allocator() throw()
		{ }

		pointer address(reference x) const
		{ return &x; }

		const_pointer address(const_reference x) const
		{ return &x; }

		pointer allocate(size_type s, void const * = 0)
		{
			if (0 == s)
				return NULL;
			pointer temp = (pointer) malloc_untracked(s * sizeof(T));
			if (temp == NULL)
				throw std::bad_alloc();
			return temp;
		}

		void deallocate(pointer p, size_type)
		{
			free_untracked(p);
		}

		size_type max_size() const throw()
		{
			return std::numeric_limits<size_t>::max() / sizeof(T);
		}

		void construct(pointer p, const T &val)
		{
			new((void *) p) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
	};

	struct MemoryInformation
	{
		const char *filename;
		size_t memory;
	};

	struct string_hash
	{
		size_t operator()(const char *s) const
		{
			size_t hash = 0;
			auto len = std::strlen(s);
			for (size_t i = 0; i < len; ++i)
				hash = 65599 * hash + s[i];
			return hash ^ (hash >> 16);
		}
	};

	struct string_equal
	{
		bool operator()(const char *s1, const char *s2) const
		{
			return std::strcmp(s1, s2) == 0;
		}
	};

	bool meminfo_sort_compare(const MemoryInformation &arg1, const MemoryInformation &arg2)
	{
		return arg1.memory > arg2.memory;
	}

	typedef std::unordered_map<const char *, MemoryInformation,
			string_hash, string_equal,
			malloc_allocator<std::pair<const char *const, MemoryInformation>>> MemoryAllocationMap;

	typedef std::vector<MemoryInformation, malloc_allocator<MemoryInformation>> MemoryAllocationVector;

	MemoryAllocationMap *allocationMap = nullptr;
	// Buffer for sorting
	MemoryAllocationVector allocationVector;

	size_t usedMemory = 0;

	std::mutex reportLock;

	void memory_uninit()
	{
#ifndef NDEBUG
		memory::debug_exit();
#endif

		if (allocationMap != nullptr)
		{
			// Need to make sure we don't call the overridden operators
			allocationMap->~MemoryAllocationMap();
			free_untracked(allocationMap);

			allocationMap = nullptr;
		}
	}
}

namespace memory
{
	const quiet_alloc_t quiet_alloc;

	void init()
	{
#ifndef NDEBUG
		memory::debug_init();

		allocationMap = new(malloc_untracked(sizeof(*allocationMap))) MemoryAllocationMap();
#endif

		std::atexit(memory_uninit);
	}

	size_t get_used_memory()
	{
		return usedMemory;
	}

	size_t sort_memory_blocks()
	{
		if (allocationMap == nullptr)
		{
			return 0;
		}

		allocationVector.clear();
		allocationVector.reserve(allocationMap->size());

		for (auto &kv : *allocationMap)
		{
			allocationVector.push_back(kv.second);
		}

		std::sort(allocationVector.begin(), allocationVector.end(), meminfo_sort_compare);

		return allocationVector.size();
	}

	bool get_memory_block(size_t index, const char *&fileName, size_t &memory)
	{
		if (index >= allocationVector.size())
		{
			return false;
		}

		auto &entry = allocationVector[index];

		fileName = entry.filename;
		memory = entry.memory;

		return true;
	}

	// Memory tracking functions
	void register_malloc(size_t size, const char *filename, int)
	{
		// MSVC <= 2013 somehow allocates memory when locking a mutex
		// MSVC >= 2015 should be fine, for earlier version this probably means 
		// that the values will be corrupted but at least they will be able to run...
#if !SCP_COMPILER_IS_MSVC || SCP_COMPILER_VERSION_MAJOR >= 19
		std::lock_guard<std::mutex> guard(reportLock);
#endif

		usedMemory += size;

		if (filename == nullptr)
		{
			// Origin not known
			filename = "<Unknown>";
		}

		if (Cmdline_show_mem_usage && allocationMap != nullptr)
		{
			auto foundIter = allocationMap->find(filename);
			if (foundIter == allocationMap->end())
			{
				MemoryInformation newVal;
				newVal.filename = filename;
				newVal.memory = 0;

				foundIter = allocationMap->insert(std::make_pair(filename, newVal)).first;
			}

			foundIter->second.memory += size;
		}
	}

	void unregister_malloc(size_t size, const char *filename, int)
	{
		// MSVC <= 2013 somehow allocates memory when locking a mutex
		// MSVC >= 2015 should be fine, for earlier version this probably means 
		// that the values will be corrupted but at least they will be able to run...
#if !SCP_COMPILER_IS_MSVC || SCP_COMPILER_VERSION_MAJOR >= 19
		std::lock_guard<std::mutex> guard(reportLock);
#endif

		usedMemory -= size;

		if (filename == nullptr)
		{
			// Origin not known
			filename = "<Unknown>";
		}

		if (Cmdline_show_mem_usage && allocationMap != nullptr)
		{
			auto foundIter = allocationMap->find(filename);
			if (foundIter == allocationMap->end())
			{
				MemoryInformation newVal;
				newVal.filename = filename;
				newVal.memory = 0;

				foundIter = allocationMap->insert(std::make_pair(filename, newVal)).first;
			}

			if (size > foundIter->second.memory)
			{
				// We freed more memory than allocated for this file
				// This can happen if one file allocates memory and another deallocates it
				foundIter->second.memory = 0;
			}
			else
			{
				foundIter->second.memory -= size;
			}
		}
	}

	void out_of_memory(const char *filename, int line)
	{
		mprintf(("Memory allocation failed!!!!!!!!!!!!!!!!!!!\n"));
		Error(filename, line, "Out of memory.  Try closing down other applications, increasing your\n"
				"virtual memory size, or installing more physical RAM.\n");
	}
}

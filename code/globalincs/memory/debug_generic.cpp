#ifndef NDEBUG

// Include this first!
#include "globalincs/memory/debug.h"

#include "globalincs/pstypes.h"

#include <cstdlib>
#include <stdlib.h>
#include <type_traits>

#ifdef HAVE_STD_MAX_ALIGN_T
#include <cstddef>
typedef std::max_align_t my_max_align_t;
#elif defined(HAVE_MAX_ALIGN_T)
#include <stddef.h>
typedef ::max_align_t my_max_align_t;
#else
#error This code requires the max_align_t type!
#endif

namespace
{
	struct alignas(my_max_align_t) MemoryHeader
	{
		const char *filename;
		size_t size;
		int line;
	};

	void* getMemoryBegin(void* user_block)
	{
		auto mem = reinterpret_cast<uint8_t*>(user_block);
		return reinterpret_cast<void*>(mem - sizeof(MemoryHeader));
	}

	void* getUserMemory(void* actual_block) {
		auto mem = reinterpret_cast<uint8_t*>(actual_block);
		return reinterpret_cast<void*>(mem + sizeof(MemoryHeader));
	}
}

namespace memory
{
	// Called to initialize memory tracking in debug builds
	void debug_init()
	{
		// Do nothing in the generic version
	}

	void debug_exit()
	{
		// Do nothing in the generic version
	}
}

void *_vm_malloc(size_t size, const memory::quiet_alloc_t &, const char *filename, int line)
{
	auto ptr = std::malloc(sizeof(MemoryHeader) + size);

	if (ptr != nullptr)
	{
		memory::register_malloc(size, filename, line);

		auto header = reinterpret_cast<MemoryHeader *>(ptr);

		header->size = size;
		header->filename = filename;
		header->line = line;

		ptr = getUserMemory(ptr);
	}

	return ptr;
}

void _vm_free(void *ptr, const char *filename, int line)
{
	if (!ptr)
	{
		return;
	}

	ptr = getMemoryBegin(ptr);

	auto header = reinterpret_cast<MemoryHeader *>(ptr);

	memory::unregister_malloc(header->size, header->filename, header->line);

	std::free(ptr);
}

void *_vm_realloc(void *ptr, size_t size, const memory::quiet_alloc_t &, const char *filename, int line)
{
	if (ptr == nullptr)
	{
		return _vm_malloc(size, filename, line);
	}

	ptr = getMemoryBegin(ptr);

	auto memoryHeader = reinterpret_cast<MemoryHeader *>(ptr);

	auto oldSize = memoryHeader->size;
	auto oldFilename = memoryHeader->filename;
	auto oldLine = memoryHeader->line;

	auto ret_ptr = std::realloc(ptr, sizeof(MemoryHeader) + size);

	if (ret_ptr == nullptr)
	{
		// realloc doesn't touch the original ptr in the case of failure so we could still use it
		return nullptr;
	}

	memory::unregister_malloc(oldSize, oldFilename, oldLine);

	auto header = reinterpret_cast<MemoryHeader *>(ret_ptr);

	header->filename = filename;
	header->line = line;
	header->size = size;

	memory::register_malloc(size, filename, line);

	return getUserMemory(ret_ptr);
}

void *malloc_untracked(size_t size)
{
	return std::malloc(size);
}

void free_untracked(void *ptr)
{
	std::free(ptr);
}

#endif

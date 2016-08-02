#ifndef NDEBUG

#include <cstdlib>

// Include this first!
#include "globalincs/memory/debug.h"

#include "globalincs/pstypes.h"

namespace
{
	struct MemoryHeader
	{
		const char *filename;
		int line;
		size_t size;
	};

	void *getMemoryBegin(void *ptr)
	{
		auto headerArray = reinterpret_cast<MemoryHeader *>(ptr);

		// The pointer should point to the beginning of the actual memory block
		// the memory header is directly "in front" of it
		return reinterpret_cast<void *>(headerArray - 1);
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

		ptr = reinterpret_cast<void *>(header + 1);
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
	auto ret_ptr = std::realloc(ptr, sizeof(MemoryHeader) + size);

	if (ret_ptr == nullptr)
	{
		// realloc doesn't touch the original ptr in the case of failure so we could still use it
		return nullptr;
	}

	memory::unregister_malloc(oldSize, memoryHeader->filename, memoryHeader->line);

	auto header = reinterpret_cast<MemoryHeader *>(ret_ptr);

	header->filename = filename;
	header->line = line;
	header->size = size;

	memory::register_malloc(size, filename, line);

	return reinterpret_cast<void *>(header + 1);
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

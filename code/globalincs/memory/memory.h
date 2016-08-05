
#pragma once

#include <stddef.h>

namespace memory
{
	void init();

	size_t get_used_memory();

	struct quiet_alloc_t { quiet_alloc_t(){} };
	extern const quiet_alloc_t quiet_alloc;

	void register_malloc(size_t size, const char *filename, int line);
	void unregister_malloc(size_t size, const char *filename, int line);

	size_t sort_memory_blocks();
	bool get_memory_block(size_t index, const char*& fileName, size_t& usedMemory);

	void out_of_memory(const char* filename, int line);
}

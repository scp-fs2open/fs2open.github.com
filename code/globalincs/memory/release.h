#pragma once

#ifndef NDEBUG
#error Release memory management header included in debug build!
#endif

#include <cstdlib>

#include "globalincs/pstypes.h"
#include "globalincs/memory/memory.h"

inline void *vm_malloc(size_t size, const memory::quiet_alloc_t &)
{ return std::malloc(size); }

inline void *vm_malloc(size_t size)
{
	auto ptr = vm_malloc(size, memory::quiet_alloc);

	if (ptr == NULL)
	{
		memory::out_of_memory("Unknown", -1);
	}

	return ptr;
}

inline void vm_free(void *ptr)
{ std::free(ptr); }

inline void *vm_realloc(void *ptr, size_t size, const memory::quiet_alloc_t &)
{ return std::realloc(ptr, size); }

inline void *vm_realloc(void *ptr, size_t size)
{
	auto ret_ptr = vm_realloc(ptr, size, memory::quiet_alloc);

	if (ret_ptr == NULL)
	{
		memory::out_of_memory("Unknown", -1);
	}

	return ret_ptr;
}

inline void* malloc_location(size_t size, const char*, int)
{
	return vm_malloc(size);
}

inline void free_location(void* ptr, const char*, int)
{
	vm_free(ptr);
}

// untracked versions DON'T USE IN NORMAL CODE!!
inline void *malloc_untracked(size_t size)
{
	return std::malloc(size);
}

inline void free_untracked(void *ptr)
{
	std::free(ptr);
}


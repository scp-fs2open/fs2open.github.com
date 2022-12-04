
#pragma once

#include <cstddef>
#include <cstdlib>

#include "globalincs/pstypes.h"

namespace memory
{
	struct quiet_alloc_t { quiet_alloc_t(){} };
	extern const quiet_alloc_t quiet_alloc;

	void out_of_memory();
}

inline void *vm_malloc(size_t size, const memory::quiet_alloc_t &)
{ return std::malloc(size); }

inline void *vm_malloc(size_t size)
{
	auto ptr = vm_malloc(size, memory::quiet_alloc);

	if (ptr == NULL)
	{
		memory::out_of_memory();
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
		memory::out_of_memory();
	}

	return ret_ptr;
}


// For use with unique_ptr
template <typename T>
struct FreeDeleter
{
	void operator()(T* const p) const
	{
		free(p);
	}
};

// For use with unique_ptr
template <typename T>
struct VmFreeDeleter
{
	void operator()(T* const p) const
	{
		vm_free(p);
	}
};

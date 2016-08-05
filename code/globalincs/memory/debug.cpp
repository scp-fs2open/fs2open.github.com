#ifndef NDEBUG

#include "debug.h"

#include "globalincs/pstypes.h"

// Generic non-quiet version that just call the specialized versions

void *_vm_malloc(size_t size, const char *filename, int line)
{
	auto ptr = _vm_malloc(size, memory::quiet_alloc, filename, line);

	if (ptr == nullptr)
	{
		memory::out_of_memory(filename, line);
	}

	return ptr;
}

void *_vm_realloc(void *ptr, size_t size, const char *filename, int line)
{
	auto ret_ptr = _vm_realloc(ptr, size, memory::quiet_alloc, filename, line);

	if (ret_ptr == nullptr)
	{
		memory::out_of_memory(filename, line);
	}

	return ret_ptr;
}

void* malloc_location(size_t size, const char* location, int line)
{
	return _vm_malloc(size, location, line);
}

void free_location(void* ptr, const char* location, int line)
{
	_vm_free(ptr, location, line);
}

#endif

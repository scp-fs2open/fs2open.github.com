
#pragma once

#ifdef NDEBUG
#error Debug memory management header included in release build!
#endif

#include <stdexcept>

#include "globalincs/memory/memory.h"
#include "globalincs/scp_defines.h"

namespace memory
{
	// Called to initialize memory tracking in debug builds
	void debug_init();

	void debug_exit();
}

// Allocates some RAM.
void *_vm_malloc(size_t size, const char *filename, int line);
void *_vm_malloc(size_t size, const memory::quiet_alloc_t&, const char *filename, int line);

// Frees some RAM.
void _vm_free(void *ptr, const char *filename, int line);

// reallocates some RAM
void *_vm_realloc(void *ptr, size_t size, const char *filename, int line);
void *_vm_realloc(void *ptr, size_t size, const memory::quiet_alloc_t&, const char *filename, int line);

// Easy to use macros
#define vm_malloc(...) _vm_malloc(__VA_ARGS__,__FILE__,__LINE__)
#define vm_free(...) _vm_free(__VA_ARGS__,__FILE__,__LINE__)
#define vm_realloc(...) _vm_realloc(__VA_ARGS__,__FILE__,__LINE__)

// Version with explicit location information
void* malloc_location(size_t size, const char* location, int line);
void free_location(void* ptr, const char* location, int line);

// untracked versions DON'T USE IN NORMAL CODE!!
void* malloc_untracked(size_t size);
void free_untracked(void* ptr);

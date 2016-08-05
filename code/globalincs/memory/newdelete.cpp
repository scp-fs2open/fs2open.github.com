
#include "globalincs/pstypes.h"

// Compile new version of new and delete which replace the ones provided by the runtime
// This will make sure that everyone uses our operators

#ifdef new
#undef new
#endif

// throw
void * operator new (size_t size)
{
	void *p = vm_malloc(size, memory::quiet_alloc);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete (void *p) SCP_NOEXCEPT
{
	vm_free(p);
}

void * operator new [] (size_t size)
{
	void *p = vm_malloc(size, memory::quiet_alloc);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete [] (void *p) SCP_NOEXCEPT
{
	vm_free(p);
}

// no-throw
void * operator new (size_t size, const std::nothrow_t&) SCP_NOEXCEPT
{
	return vm_malloc(size, memory::quiet_alloc);
}

void operator delete (void *p, const std::nothrow_t&) SCP_NOEXCEPT
{
	vm_free(p);
}

void * operator new [] (size_t size, const std::nothrow_t&) SCP_NOEXCEPT
{
	return vm_malloc(size, memory::quiet_alloc);
}

void operator delete [] (void *p, const std::nothrow_t&) SCP_NOEXCEPT
{
	vm_free(p);
}

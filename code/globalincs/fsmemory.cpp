
#include "globalincs/pstypes.h"


// throw
#ifdef HAVE_CXX11
void * operator new (size_t size)
#else
void * operator new (size_t size) throw (std::bad_alloc)
#endif // CPP11_STD
{
	void *p = vm_malloc_q(size);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete (void *p) throw()
{
	vm_free(p);
}

#ifdef HAVE_CXX11
void * operator new [] (size_t size)
#else
void * operator new [] (size_t size) throw (std::bad_alloc)
#endif // CPP11_STD
{
	void *p = vm_malloc_q(size);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete [] (void *p) throw()
{
	vm_free(p);
}

// no-throw
void * operator new (size_t size, const std::nothrow_t&) throw()
{
	return vm_malloc_q(size);
}

void operator delete (void *p, const std::nothrow_t&) throw()
{
	vm_free(p);
}

void * operator new [] (size_t size, const std::nothrow_t&) throw()
{
	return vm_malloc_q(size);
}

void operator delete [] (void *p, const std::nothrow_t&) throw()
{
	vm_free(p);
}

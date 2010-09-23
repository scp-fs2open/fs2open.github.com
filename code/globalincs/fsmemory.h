
#ifndef _FSMEMORY_H
#define _FSMEMORY_H

#include <new>

inline void * operator new (size_t size) throw (std::bad_alloc)
{
	void *p = vm_malloc_q(size);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

inline void operator delete (void *p) throw()
{
	vm_free(p);
}

inline void * operator new [] (size_t size) throw (std::bad_alloc)
{
	void *p = vm_malloc_q(size);

	if ( !p ) {
		throw std::bad_alloc();
	}

	return p;
}

inline void operator delete [] (void *p) throw()
{
	vm_free(p);
}

inline void * operator new (size_t size, const std::nothrow_t&) throw()
{
	return vm_malloc_q(size);
}

inline void operator delete (void *p, const std::nothrow_t&) throw()
{
	vm_free(p);
}

inline void * operator new [] (size_t size, const std::nothrow_t&) throw()
{
	return vm_malloc_q(size);
}

inline void operator delete [] (void *p, const std::nothrow_t&) throw()
{
	vm_free(p);
}

#endif	// _FSMEMORY_H

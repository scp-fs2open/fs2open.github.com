
#ifndef _FSMEMORY_H
#define _FSMEMORY_H

#include <new>

// throw
extern void * operator new (size_t size) throw (std::bad_alloc);

extern void operator delete (void *p) throw();

extern void * operator new [] (size_t size) throw (std::bad_alloc);

extern void operator delete [] (void *p) throw();

// no-throw
extern void * operator new (size_t size, const std::nothrow_t&) throw();

extern void operator delete (void *p, const std::nothrow_t&) throw();

extern void * operator new [] (size_t size, const std::nothrow_t&) throw();

extern void operator delete [] (void *p, const std::nothrow_t&) throw();

#endif	// _FSMEMORY_H

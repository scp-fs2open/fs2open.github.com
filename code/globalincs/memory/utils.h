#pragma once

#include <cstring>

#include "globalincs/pstypes.h"

#ifndef NDEBUG
inline char *_vm_strndup(const char *ptr, size_t size, const char *filename, int line)
#else
inline char *vm_strndup(const char *ptr, size_t size)
#endif
{
#ifndef NDEBUG
	char *dst = (char *) _vm_malloc(size + 1, filename, line);
#else
	char *dst = (char *) vm_malloc(size + 1);
#endif

	if (!dst)
		return NULL;

	std::strncpy(dst, ptr, size);
	// make sure it has a NULL terminiator
	dst[size] = '\0';

	return dst;
}

#ifndef NDEBUG
inline char *_vm_strdup(const char *ptr, const char *filename, int line)
#else
inline char *vm_strdup(const char *ptr)
#endif
{
	size_t len = std::strlen(ptr);

#ifndef NDEBUG
	return _vm_strndup(ptr, len, filename, line);
#else
	return vm_strndup(ptr, len);
#endif
}

#ifndef NDEBUG
#define vm_strdup(ptr) _vm_strdup((ptr),__FILE__,__LINE__)
#define vm_strndup(ptr, size) _vm_strndup((ptr),(size),__FILE__,__LINE__)
#endif

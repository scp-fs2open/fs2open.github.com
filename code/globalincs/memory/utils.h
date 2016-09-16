#pragma once

#include <cstring>

#include "globalincs/pstypes.h"

inline char *vm_strndup(const char *ptr, size_t size)
{
	char *dst = (char *) vm_malloc(size + 1);

	if (!dst)
		return NULL;

	std::strncpy(dst, ptr, size);
	// make sure it has a NULL terminiator
	dst[size] = '\0';

	return dst;
}

inline char *vm_strdup(const char *ptr)
{
	size_t len = std::strlen(ptr);

	return vm_strndup(ptr, len);
}

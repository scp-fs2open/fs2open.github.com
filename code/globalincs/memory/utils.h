#pragma once

#include <cstring>

inline char *vm_strndup(const char *ptr, size_t len)
{
	char *dst = static_cast<char *>(vm_malloc(len + 1));

	if (!dst)
		return nullptr;

	std::strncpy(dst, ptr, len);
	// make sure it has a NULL terminiator
	dst[len] = '\0';

	return dst;
}

inline char *vm_strdup(const char *ptr)
{
	size_t len = std::strlen(ptr);

	return vm_strndup(ptr, len);
}

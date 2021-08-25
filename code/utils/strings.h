//
//

#ifndef FS2_OPEN_CASECMP_H
#define FS2_OPEN_CASECMP_H

#include "platformChecks.h"

#include <utility>

#if SCP_HAVE_STRCASECMP || SCP_HAVE_STRNCASECMP
#if SCP_HAVE_STRINGS_H
#include <strings.h>
#else
#error strings.h is not available!
#endif
#endif

#if SCP_HAVE__STRICMP || SCP_HAVE__STRNICMP || SCP_HAVE_STRLWR
#include <string.h>
#endif

#if SCP_HAVE_STRCASECMP
inline int stricmp(const char* s1, const char* s2) {
	return strcasecmp(s1, s2);
}
#elif SCP_HAVE__STRICMP
inline int stricmp(const char* s1, const char* s2) {
	return _stricmp(s1, s2);
}
#else
#error No case insensitive string comparision available!
#endif

#if SCP_HAVE_STRNCASECMP
inline int strnicmp(const char* s1, const char* s2, size_t n) {
	return strncasecmp(s1, s2, n);
}
#elif SCP_HAVE__STRNICMP
inline int strnicmp(const char* s1, const char* s2, size_t n) {
	return _strnicmp(s1, s2, n);
}
#else
#error No case insensitive string comparision available!
#endif

#if !SCP_HAVE_STRLWR
inline void strlwr(char *s) {
	if (s == NULL)
		return;

	while (*s) {
		*s = (char) tolower((unsigned char) *s);
		s++;
	}
}
#endif

#if !SCP_HAVE_SNPRINTF
// Older Visual Studio versions have _snprintf but that does not always append a null terminator so we do not want to
// use that
#error No support for standard conforming snprintf detected!
#endif

template<size_t SIZE, typename... Args>
inline int sprintf_safe(char (&dest)[SIZE], const char* format, Args&&... args) {
	auto written = snprintf(dest, SIZE, format, std::forward<Args>(args)...);

	if (written < 0) {
		return written;
	}

	if ((size_t)written >= SIZE) {
		dest[SIZE - 1] = '\0';
	}
	return written;
}

#endif //FS2_OPEN_CASECMP_H

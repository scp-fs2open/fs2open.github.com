//
//

#ifndef FS2_OPEN_CASECMP_H
#define FS2_OPEN_CASECMP_H

#include "platformChecks.h"

#include <utility>

#if HAVE_STRCASECMP || HAVE_STRNCASECMP
#if HAVE_STRINGS_H
#include <strings.h>
#else
#error strings.h is not available!
#endif
#endif

#if HAVE__STRICMP || HAVE__STRNICMP || HAVE_STRLWR
#include <string.h>
#endif

#if HAVE_STRCASECMP
inline int stricmp(const char* s1, const char* s2) {
	return strcasecmp(s1, s2);
}
#elif HAVE__STRICMP
inline int stricmp(const char* s1, const char* s2) {
	return _stricmp(s1, s2);
}
#else
#error No case insensitive string comparision available!
#endif

#if HAVE_STRNCASECMP
inline int strnicmp(const char* s1, const char* s2, size_t n) {
	return strncasecmp(s1, s2, n);
}
#elif HAVE__STRNICMP
inline int strnicmp(const char* s1, const char* s2, size_t n) {
	return _strnicmp(s1, s2, n);
}
#else
#error No case insensitive string comparision available!
#endif

#if !HAVE_STRLWR
inline void strlwr(char *s) {
	if (s == NULL)
		return;

	while (*s) {
		*s = (char) tolower(*s);
		s++;
	}
}
#endif

#if !HAVE_SNPRINTF
#if HAVE__SNPRINTF
#define snprintf _snprintf
#else
#error No support for snprintf detected!
#endif
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

//
//

#ifndef FS2_OPEN_CASECMP_H
#define FS2_OPEN_CASECMP_H

#include "platformChecks.h"

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

#endif //FS2_OPEN_CASECMP_H

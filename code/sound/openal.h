
#ifndef _AL_H
#define _AL_H


#if !(defined(__APPLE__) || defined(_WIN32))
	#include <AL/al.h>
	#include <AL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif // !__APPLE__ && !_WIN32

#include <string>


const char* openal_error_string(int get_alc = 0);
bool openal_init_device(std::string *playback, std::string *capture);


// if an error occurs after executing 'x' then do 'y'
#define OpenAL_ErrorCheck( x, y )	do {	\
	x;	\
	const char *error_text = openal_error_string(0);	\
	if ( error_text != NULL ) {	\
		nprintf(("Warning", "SOUND: %s:%d - OpenAL error = '%s'\n", __FILE__, __LINE__, error_text));	\
		y;	\
	}	\
} while (0);

// like OpenAL_ErrorCheck() except that it gives the error message from x but does nothing about it
#define OpenAL_ErrorPrint( x )	do {	\
	x;	\
	const char *error_text = openal_error_string(0);	\
	if ( error_text != NULL ) {	\
		nprintf(("Sound", "OpenAL ERROR: \"%s\" in %s, line %i\n", error_text, __FILE__, __LINE__));	\
	}	\
} while (0);

// same as the above two, but looks for ALC errors instead of standard AL errors
#define OpenAL_C_ErrorCheck( x, y )	do {	\
	x;	\
	const char *error_text = openal_error_string(1);	\
	if ( error_text != NULL ) {	\
		nprintf(("Warning", "SOUND: %s:%d - OpenAL error = '%s'\n", __FILE__, __LINE__, error_text));	\
		y;	\
	}	\
} while (0);

// like OpenAL_ErrorCheck() except that it gives the error message from x but does nothing about it
#define OpenAL_C_ErrorPrint( x )	do {	\
	x;	\
	const char *error_text = openal_error_string(1);	\
	if ( error_text != NULL ) {	\
		nprintf(("Sound", "OpenAL ERROR: \"%s\" in %s, line %i\n", error_text, __FILE__, __LINE__));	\
	}	\
} while (0);


#endif	// _AL_H

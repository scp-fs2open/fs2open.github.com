/**
 * Coverity Scan modeling file.  Help reduce false positives
 */

// Assert & Assertion are "killpaths" in DEBUG, but not release
// This should get them treated as killpaths in release as well
// (just a copy of the DEBUG version of the macros)

void WinAssert(char * text, char *filename, int line)
{
	__coverity_panic__();
}

void WinAssert(char * text, char *filename, int line, const char * format, ... )
{
	__coverity_panic__();
}

#	define Assert(expr) do {\
		if (!(expr)) {\
			WinAssert(#expr,__FILE__,__LINE__);\
		}\
		ASSUME( expr );\
	} while (0)

#	ifndef _MSC_VER   // non MS compilers
#		define Assertion(expr, msg, ...) do {\
			if (!(expr)) {\
				WinAssert(#expr,__FILE__,__LINE__, msg , ##__VA_ARGS__ );\
			}\
		} while (0)
#	else
#		if _MSC_VER >= 1400	// VC 2005 or greater
#			define Assertion(expr, msg, ...) do {\
				if (!(expr)) {\
					WinAssert(#expr,__FILE__,__LINE__, msg, __VA_ARGS__ );\
				}\
				ASSUME(expr);\
			} while (0)
#		else // older MSVC compilers
#			define Assertion(expr, msg) do {\
				if (!(expr)) {\
					WinAssert(#expr,__FILE__,__LINE__);\
				}\
			} while (0)
#		endif
#	endif



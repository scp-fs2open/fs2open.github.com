/**
 * Coverity Scan modeling file.  Help reduce false positives
 */

/*
 * In theory these fatal errors can allow execution to continue, because we give an option to open the debugger when they occur
 * However, Coverity should treat these as killpaths, since users should never pick the debug option
 * These two should cover Error, Assert & Assertion (and maybe others, I haven't done an exhaustive check)
 */
namespace os
{
	namespace dialogs
	{
		void Error(const char* text)
		{
			__coverity_panic__();
		}

		void AssertMessage(const char * text, const char * filename, int linenum, const char * format, ...)
		{
			__coverity_panic__();
		}
	}
}

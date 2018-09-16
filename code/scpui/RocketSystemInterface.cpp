//
//

#include "RocketSystemInterface.h"

#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "localization/localize.h"

using namespace Rocket::Core;

namespace scpui {

float RocketSystemInterface::GetElapsedTime() { return timer_get_microseconds() / 1000000.f; }
int RocketSystemInterface::TranslateString(Rocket::Core::String& translated, const Rocket::Core::String& input)
{
	SCP_string lcl_translated;
	lcl_ext_localize(input.CString(), lcl_translated);

	translated = lcl_translated.c_str();

	return 0; // Everything is translated now
}
void RocketSystemInterface::JoinPath(Rocket::Core::String& translated_path,
                                     const Rocket::Core::String& /*document_path*/, const Rocket::Core::String& path)
{
	// We here at the SCP are rather simple. We just take the path that is in the document.
	// Our file opening procedures take care of finding those files
	translated_path = path;
}
bool RocketSystemInterface::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
{
	switch (type) {
	case Log::LT_ERROR:
		ReleaseWarning(LOCATION, "libRocket error: %s", message.CString());
		break;
	case Log::LT_WARNING:
		Warning(LOCATION, "libRocket warning: %s", message.CString());
		break;
	case Log::LT_ASSERT:
		os::dialogs::AssertMessage("libRocket assertion", LOCATION, "%s", message.CString());
		break;
	case Log::LT_ALWAYS:
	case Log::LT_INFO:
	case Log::LT_DEBUG:
		// These always go into the standard log
		mprintf(("libRocket message: %s\n", message.CString()));
		break;
	default:
		break;
	}
	// We never break into the debugger
	return false;
}
void RocketSystemInterface::ActivateKeyboard()
{
	// TODO: Maybe extend libRocket to expose where the input currently is
	SDL_StartTextInput();
}
void RocketSystemInterface::DeactivateKeyboard() { SDL_StopTextInput(); }

} // namespace scpui

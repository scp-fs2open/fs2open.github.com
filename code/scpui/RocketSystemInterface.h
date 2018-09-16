#pragma once

#include "globalincs/pstypes.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/SystemInterface.h>

#pragma pop_macro("Assert")

namespace scpui {

/**
 * @brief Implementation of the rocket system interface for FSO
 */
class RocketSystemInterface : public Rocket::Core::SystemInterface {
  public:
	/**
	 * @details Uses the FSO timer routines for determining how much time has passed
	 *
	 * @return
	 */
	float GetElapsedTime() override;

	/**
	 * @details Translates the given string using the FSO translation mechanics. This will replace string of the form
	 * "XSTR("Test", 1234)" with the corresponding string that was defined in the tables. It will also replace variables
	 * such as $callsign
	 *
	 * @param translated
	 * @param input
	 * @return
	 */
	int TranslateString(Rocket::Core::String& translated, const Rocket::Core::String& input) override;

	/**
	 * @details Since the FSO file opening implementation already takes care of locating a specific file, this
	 * implementation simply returns the path that was specified in the document without altering it.
	 *
	 * @param translated_path
	 * @param document_path
	 * @param path
	 */
	void JoinPath(Rocket::Core::String& translated_path, const Rocket::Core::String& document_path,
	              const Rocket::Core::String& path) override;

	/**
	 * @details Logs a message using the FSO logging and message routines. Errors, Warnings, and Assertions will use the
	 * respective dialogs while logging and debug messages will be directed to the FSO log.
	 *
	 * @param type
	 * @param message
	 * @return
	 */
	bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message) override;

	/**
	 * @details Activates the keyboard by using the SDL provided functions for that.
	 */
	void ActivateKeyboard() override;

	/**
	 * @details Also uses the SDL functions for deactivating the keyboard again
	 */
	void DeactivateKeyboard() override;
};

} // namespace scpui

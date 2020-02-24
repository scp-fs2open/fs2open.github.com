#pragma once
//
//

#include "globalincs/pstypes.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/FileInterface.h>

#pragma pop_macro("Assert")

namespace scpui {

class RocketFileInterface : public Rocket::Core::FileInterface {
  public:
	RocketFileInterface();

	/**
	 * @brief Opens the specified file using the CFile system
	 *
	 * @details The path is split into a directory path and a file name. If the directory path is not valid then the
	 * file will not be opened.
	 *
	 * @param path
	 * @return
	 */
	Rocket::Core::FileHandle Open(const Rocket::Core::String& path) override;

	void Close(Rocket::Core::FileHandle file) override;

	size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file) override;

	bool Seek(Rocket::Core::FileHandle file, long offset, int origin) override;

	size_t Tell(Rocket::Core::FileHandle file) override;

	size_t Length(Rocket::Core::FileHandle file) override;

	static bool getCFilePath(const Rocket::Core::String& path, SCP_string& name, int& dir_type);
};

} // namespace scpui

//
//

#include "RocketFileInterface.h"

#include "cfile/cfile.h"
#include "mod_table/mod_table.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Log.h>

#pragma pop_macro("Assert")

using namespace Rocket::Core;

namespace scpui {

RocketFileInterface::RocketFileInterface() = default;
FileHandle RocketFileInterface::Open(const String& path)
{
	// In order to use libRocket correctly we need to make sure that we only have Unicode data. This is done here since
	// a mod that uses libRocket will always open a file through this function but a mod that doesn't will not call this
	if (!Unicode_text_mode) {
		Error(LOCATION,
		      "libRocket was used without Unicode text mode being enabled! The new UI system requires Unicode text.");
	}

	int dir_type;
	SCP_string name;

	if (!getCFilePath(path, name, dir_type)) {
		return 0;
	}

	return (FileHandle)cfopen(name.c_str(), "rb", CFILE_NORMAL, dir_type);
}
void RocketFileInterface::Close(FileHandle file) { cfclose((CFILE*)file); }
size_t RocketFileInterface::Read(void* buffer, size_t size, FileHandle file)
{
	auto fp = (CFILE*)file;

	auto read = cfread(buffer, 1, (int)size, fp);

	return (size_t)read;
}
bool RocketFileInterface::Seek(FileHandle file, long offset, int origin)
{
	auto fp = (CFILE*)file;

	int cf_seek_mode;
	switch (origin) {
	case SEEK_SET:
		cf_seek_mode = CF_SEEK_SET;
		break;
	case SEEK_CUR:
		cf_seek_mode = CF_SEEK_CUR;
		break;
	case SEEK_END:
		cf_seek_mode = CF_SEEK_END;
		break;
	default:
		Assertion(false, "Invalid seek mode encountered!");
		return false;
	}

	return cfseek(fp, (int)offset, cf_seek_mode) == 0;
}
size_t RocketFileInterface::Tell(FileHandle file) { return (size_t)cftell((CFILE*)file); }
size_t RocketFileInterface::Length(FileHandle file) { return (size_t)cfilelength((CFILE*)file); }
bool RocketFileInterface::getCFilePath(const String& path_in, SCP_string& name, int& dir_type)
{
	String path = path_in;
	if (path.Find("\\") != String::npos) {
		// We don't allow these in our paths
		Log::Message(
		    Log::LT_WARNING,
		    "The path '%s' contains the invalid character '\\'. This is not allowed! You should use '/' instead.",
		    path.CString());
		path.Replace("\\", "/");
	}

	if (path.Length() > 0 && path[path.Length() - 1] == '/') {
		// Trailing slashes are not allowed!
		Log::Message(Log::LT_WARNING,
		             "The path '%s' has trailing slashes. This is not allowed! The trailing slashes have been removed.",
		             path.CString());

		while (path.Length() > 0 && path[path.Length() - 1] == '/') {
			path = path.Substring(0, path.Length() - 1);
		}
	}

	auto slashPos = path.RFind("/");

	if (slashPos == String::npos) {
		// No path specified, search everywhere
		dir_type = CF_TYPE_ANY;
		name     = path.CString();
		return true;
	}

	auto dir_path = path.Substring(0, slashPos);

	auto type = cfile_get_path_type(dir_path.CString());

	if (type == CF_TYPE_INVALID) {
		return false;
	}

	dir_type = type;
	name     = path.Substring(slashPos + 1).CString();

	return true;
}
} // namespace scpui

/*
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "def_files/def_files.h"

#include <iterator>

struct def_file
{
	const char* path_type;
	const char* filename;
	const void* contents;
	const size_t size;
};

#include "embedded_files.inc"

default_file defaults_get_file(const char *filename)
{
	default_file def{};

	auto endIter = std::end(Default_files);
	for (auto iter = std::begin(Default_files); iter != endIter; ++iter)
	{
		if (!stricmp(iter->filename, filename))
		{
			def.path_type = iter->path_type;
			def.filename = iter->filename;
			def.data = iter->contents;
			def.size = iter->size;

			return def;
		}
	}

	//WMC - This is really bad, because it means we have a default table missing.
	Error(LOCATION, "Default table '%s' missing from executable - contact a coder.", filename);
	return def;
}

SCP_vector<default_file> defaults_get_all() {
	SCP_vector<default_file> files;

	for (const auto& file : Default_files) {
		default_file def;
		def.path_type = file.path_type;
		def.filename = file.filename;
		def.data = file.contents;
		def.size = file.size;

		files.push_back(def);
	}

	return files;
}

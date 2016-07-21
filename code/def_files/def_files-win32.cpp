/*
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "def_files/def_files.h"

#include <iterator>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct def_file
{
	const char* filename;
	const TCHAR* resource_name;
};

def_file Default_files[] =
{
#include "def_files/generated_def_files-win32.h"
};

default_file defaults_get_file(const char *filename)
{
	default_file def;

	auto endIter = std::end(Default_files);
	for (auto iter = std::begin(Default_files); iter != endIter; ++iter)
	{
		if (!stricmp(iter->filename, filename))
		{
			HRSRC resource = FindResource(nullptr, iter->resource_name, RT_RCDATA);

			if (resource == nullptr)
			{
				continue;
			}

			HGLOBAL resHandle = LoadResource(nullptr, resource);

			if (resHandle == nullptr)
			{
				continue;
			}

			def.data = LockResource(resHandle);
			def.size = SizeofResource(nullptr, resource);

			return def;
		}
	}

	//WMC - This is really bad, because it means we have a default table missing.
	Error(LOCATION, "Default table '%s' missing from executable - contact a coder.", filename);

	def.data = nullptr;
	def.data = 0;
	return def;
}

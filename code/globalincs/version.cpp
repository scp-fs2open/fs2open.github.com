/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "globalincs/version.h"

#include <sstream>

namespace version
{
	bool check_at_least(int major, int minor, int build, int revision)
	{
		if (FS_VERSION_MAJOR < major)
		{
			return false;
		}
		if (FS_VERSION_MAJOR > major)
		{
			// Major is greater than the given version => the rest doesn't matter
			return true;
		}
		// major is now equal to our major version
		
		if (FS_VERSION_MINOR < minor)
		{
			return false;
		}
		if (FS_VERSION_MINOR > minor)
		{
			// Minor is greater than the given version => the rest doesn't matter
			return true;
		}
		// minor is now equal to our minor version
		
		if (FS_VERSION_BUILD < build)
		{
			return false;
		}
		if (FS_VERSION_BUILD > build)
		{
			// build is greater than the given version => the rest doesn't matter
			return true;
		}
		// build is now equal to our build version
		
		if (revision == 0)
		{
			// Special case, if there is no revision info, skip it
			return true;
		}
		if (FS_VERSION_REVIS == 0)
		{
			// Special case, when there is no revision ignore it
			return true;
		}
		
		if (FS_VERSION_REVIS < revision)
		{
			return false;
		}
		if (FS_VERSION_REVIS > revision)
		{
			// build is greater than the given version => the rest doesn't matter
			return true;
		}
		
		// revision is now equal to our revision version
		return true;
	}
	
	SCP_string format_version(int major, int minor, int build, int revision)
	{
		SCP_stringstream ss;
		
		ss << major << "." << minor << "." << build;
		
		if (revision != 0)
		{
			ss << "." << revision;
		}
		
		return ss.str();
	}
}


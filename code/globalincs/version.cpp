/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/version.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"

#include "parse/parselo.h"

namespace gameversion {

bool check_at_least(const version& v) {
	return get_executable_version() >= v;
}

SCP_string format_version(const version& v, bool exclude_build) {
	SCP_stringstream ss;

	ss << v.major << "." << v.minor;

	if (!exclude_build) {
		ss << "." << v.build;

		if (v.revision != 0) {
			ss << "." << v.revision;
		}
	}

	return ss.str();
}
version parse_version() {
	version v;

	required_string("+Major:");
	stuff_int(&v.major);

	required_string("+Minor:");
	stuff_int(&v.minor);

	required_string("+Build:");
	stuff_int(&v.build);

	if (optional_string("+Revision:")) {
		stuff_int(&v.revision);
	}

	return v;
}
version parse_version_inline() {
	SCP_string input;
	stuff_string(input, F_RAW);

	return version(std::move(input), 0);
}
version get_executable_version() {
	version v;
	v.major = FS_VERSION_MAJOR;
	v.minor = FS_VERSION_MINOR;
	v.build = FS_VERSION_BUILD;
	v.revision = FS_VERSION_REVIS;

	return v;
}

version::version(int major_in, int minor_in, int build_in, int revision_in) :
	major(major_in), minor(minor_in), build(build_in), revision(revision_in) {
}

version::version(const SCP_string& semver, int missing) : major(missing), minor(missing), build(missing), revision(missing) {
	int i = 0;
	auto str = semver;
	int has_fields = 0;
	while (i <= 4) {
		size_t pos = str.find_first_of('.');
		SCP_string ver;
		if (pos != SCP_string::npos) {
			ver = str.substr(0, pos);
			str.erase(0, pos + 1);
		}
		else {
			ver = str;
			str.clear();
		}

		if (!ver.empty() && std::find_if(ver.begin(), ver.end(), [](char c) { return !std::isdigit(c, SCP_default_locale); }) == ver.end()) {
			switch (has_fields) {
			case 0:
				major = std::stoi(ver.c_str());
				++has_fields;
				break;
			case 1:
				minor = std::stoi(ver.c_str());
				++has_fields;
				break;
			case 2:
				build = std::stoi(ver.c_str());
				++has_fields;
				break;
			case 3:
				revision = std::stoi(ver.c_str());
			++has_fields;
				break;
			default:
				UNREACHABLE("Version parsing broken, get a coder!");
				break;
			}
		}
		else 
			break; //Break out of the loop if the first string is not a digit.
		
		i++;
	}
}

bool version::isValid() const { return major != 0 || minor != 0 || build != 0 || revision != 0; }
bool version::operator<(const version& v) const {

	if (major < v.major) {
		return true;
	}
	if (major > v.major) {
		// Major is greater than the given version => the rest doesn't matter
		return false;
	}
	// major is now equal to our major version

	if (minor < v.minor) {
		return true;
	}
	if (minor > v.minor) {
		// Minor is greater than the given version => the rest doesn't matter
		return false;
	}
	// minor is now equal to our minor version

	if (build < v.build) {
		return true;
	}
	if (build > v.build) {
		// build is greater than the given version => the rest doesn't matter
		return false;
	}
	// build is now equal to our build version

	if (revision == 0 || v.revision == 0) {
		// Special case, if there is no revision info, then the versions are equal which means they are not less than each other
		return false;
	}

	if (revision < v.revision) {
		return true;
	}
	if (revision > v.revision) {
		// build is greater than the given version => the rest doesn't matter
		return false;
	}

	// revision is now equal to our revision version
	return false;
}
bool version::operator==(const version& other) const {
	if (major == other.major && minor == other.minor && build == other.build) {
		if (revision == 0 || other.revision == 0) {
			// Zero revisions always means that these are equal
			return true;
		}

		return revision == other.revision;
	}

	return false;
}
bool version::operator>(const version& other) const {
	return other < *this;
}
bool version::operator<=(const version& other) const {
	return !(other < *this);
}
bool version::operator>=(const version& other) const {
	return !(*this < other);
}
bool version::operator!=(const version& other) const {
	return !(*this == other);
}

SCP_string get_version_string()
{
	SCP_string str;

	sprintf(str, "FreeSpace 2 Open v%s", FS_VERSION_FULL);

#ifndef NDEBUG
	str += " Debug";
#endif

	// Lets get some more info in here
	switch (gr_screen.mode) {
	case GR_OPENGL:
		str += " OpenGL";
		#ifdef USE_OPENGL_ES
		str += " ES";
		#endif
		break;
	case GR_VULKAN:
		str += " Vulkan";
		break;
	}

	return str;
}
}


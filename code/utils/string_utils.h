#pragma once

#include "globalincs/pstypes.h"

namespace util {

template <typename Out>
void split_string(const SCP_string& s, char delim, Out result)
{
	SCP_stringstream ss(s);
	SCP_string item;
	while (std::getline(ss, item, delim)) {
		if (!item.empty()) {
			*(result++) = item;
		}
	}
}

std::vector<std::string> split_string(const std::string& s, char delim);

// get a filename minus any leading path
template <typename T>
T *get_file_part(T *path)
{
	T *p = path + strlen(path)-1;

	// Move p to point to first character of filename (check both types of dir separator)
	while( (p >= path) && (*p != '\\') && (*p != '/') && (*p != ':') )
		p--;

	p++;

	return p;
}

} // namespace util

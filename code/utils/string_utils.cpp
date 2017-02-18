//
//

#include "string_utils.h"


namespace util {

std::vector<std::string> split_string(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split_string(s, delim, std::back_inserter(elems));
	return elems;
}

}

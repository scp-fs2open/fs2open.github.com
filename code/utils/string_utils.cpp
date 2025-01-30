//
//

#include "string_utils.h"


namespace util {

std::vector<std::string> split_string(const std::string& s, char delim)
{
	std::vector<std::string> elems;
	split_string(s, delim, std::back_inserter(elems));
	return elems;
}

bool isStringOneOf(const std::string& value, const std::vector<std::string>& candidates)
{
	return std::any_of(candidates.begin(), candidates.end(), [&value](const std::string& candidate) {
		return lcase_equal(value, candidate);
	});
}

} // namespace util
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

std::unique_ptr<char[]> unique_copy(const char *str, bool null_if_empty)
{
	size_t len;

	if (str == nullptr || (len = strlen(str), null_if_empty && len == 0))
		return {};

	auto copy = std::make_unique<char[]>(len + 1);
	strcpy(copy.get(), str);
	return copy;
}

SCP_vm_unique_ptr<char> vm_unique_copy(const char *str, bool null_if_empty)
{
	size_t len;

	if (str == nullptr || (len = strlen(str), null_if_empty && len == 0))
		return {};

	return SCP_vm_unique_ptr<char>(vm_strdup(str));
}

}

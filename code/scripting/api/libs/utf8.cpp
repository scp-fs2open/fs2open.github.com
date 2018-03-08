#include "utf8.h"

#include "mod_table/mod_table.h"
#include "utils/unicode.h"

namespace scripting {
namespace api {

// This function was taken from the lua source to maintain compatibility
static ptrdiff_t resolve_relative_position(ptrdiff_t pos, size_t len) {
	/* relative string position: negative means back from end */
	if (pos < 0) {
		pos += (ptrdiff_t) len + 1;
	}
	return (pos >= 0) ? pos : 0;
}

//*************************Unicode stuff*************************
ADE_LIB(l_Utf8, "Unicode", "utf8", "Functions for handling UTF-8 encoded unicode strings");

ADE_FUNC(sub,
		 l_Utf8,
		 "string arg, number start[, number end = -1]",
		 "This function is similar to the standard library string.sub but this can operate on UTF-8 encoded unicode strings. "
			 "This function will respect the unicode mode setting of the current mod so you can use it even if you don't use Unicode strings.",
		 "string",
		 "The requestd substring") {
	const char* string = nullptr;
	int start = -1;
	int end = -1;
	if (!ade_get_args(L, "si|i", &string, &start, &end)) {
		return ADE_RETURN_NIL;
	}
	auto end_ptr = string + strlen(string);
	auto length = unicode::num_codepoints(string, end_ptr);

	auto normalized_start = resolve_relative_position(start, length);
	auto normalized_end = resolve_relative_position(end, length);

	if (normalized_start < 1) {
		normalized_start = 1;
	}
	if (normalized_end > (ptrdiff_t) length) {
		normalized_end = (ptrdiff_t) length;
	}
	if (normalized_start > normalized_end) {
		return ade_set_args(L, "s", "");
	}
	SCP_string result_str;
	size_t current = 1;
	auto inserter = std::back_inserter(result_str);
	for (auto cp : unicode::codepoint_range(string, end_ptr)) {
		if ((ptrdiff_t) current >= normalized_start && (ptrdiff_t) current <= normalized_end) {
			unicode::encode(cp, inserter);
		}

		++current;
	}

	return ade_set_args(L, "s", result_str.c_str());
}

ADE_FUNC(len,
		 l_Utf8,
		 "string arg",
		 "Determines the number of codepoints in the given string. This respects the unicode mode setting of the mod.",
		 "number",
		 "The number of code points in the string.") {
	const char* string = nullptr;
	if (!ade_get_args(L, "s", &string)) {
		return ADE_RETURN_NIL;
	}

	auto length = unicode::num_codepoints(string, string + strlen(string));

	return ade_set_args(L, "i", (int)length);
}
}
}

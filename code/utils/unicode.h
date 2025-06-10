#pragma once

#include "globalincs/pstypes.h"

#include "mod_table/mod_table.h"

#include <iterator>
#include <cinttypes>

#include <utf8.h>

#if !HAVE_CHAR32_T
// Older compilers don't have this as a built-in type so we define it for those
typedef std::uint_least32_t char32_t;
#endif

#if !HAVE_UNICODE_CHAR_LITERAL
#define UNICODE_CHAR(c) (char32_t) c
#else
/**
 * @brief Compatibility macro for compilers which don't support U'' char literals.
 *
 * @warning This can only handle standard ASCII character values since older compilers still use standard character
 * literals
 */
#define UNICODE_CHAR(c) U##c
#endif

namespace unicode {

/**
 * @brief A standard unicode codepoint
 */
typedef char32_t codepoint_t;

class text_iterator {
	const char* current_byte = nullptr;
	const char* range_end_byte = nullptr;
	const char* range_start_byte = nullptr;

	bool is_from_same_range(const text_iterator& other) const;
 public:
	explicit text_iterator(const char* current_byte, const char* range_start_byte, const char* range_end_byte = nullptr);

	typedef codepoint_t value_type;

	const char* pos() const;

	text_iterator& operator++();
	text_iterator& operator--();

	value_type operator*();

	bool operator==(const text_iterator& rhs) const;
	bool operator!=(const text_iterator& rhs) const;

	bool operator<(const text_iterator& rhs) const;
	bool operator>(const text_iterator& rhs) const;
	bool operator<=(const text_iterator& rhs) const;
	bool operator>=(const text_iterator& rhs) const;

	text_iterator operator+(ptrdiff_t diff) const;
	text_iterator operator-(ptrdiff_t diff) const;
};

/**
 * @brief Represents a range of unicode codepoints that can be iterated over
 *
 * @note This class can be used in range based for loops
 */
class codepoint_range {
	const char* start = nullptr;
	const char* end_ptr = nullptr;

 public:
	/**
	 * @brief Creates a codepoint range based on an UTF-8 encoded string
	 * @param start The start of the encoded string
	 * @param end The end of the encoded string. May be @c nullptr in which case @c start is assumed to be null-terminated
	 */
	explicit codepoint_range(const char* start, const char* end = nullptr);

	/**
	 * @brief Retrieves an iterator for the start of the codepoint range
	 * @return An iterator which is located at the start of the range
	 */
	text_iterator begin();

	/**
	 * @brief Retrieves an iterator for the end of the codepoint range
	 * @return An iterator which is located at the end of the range
	 */
	text_iterator end();
};

/**
 * @brief Computes the byte size the given codepoint would have if it were encoded using the standard encoding
 *
 * The standard encoding is UTF-8 in Unicode mode and ASCII otherwise.
 *
 * @param cp The codepoint to determine the size for.
 * @return The number of bytes required for encoding this code point.
 */
size_t encoded_size(codepoint_t cp);

/**
 * @brief Appends the given code point to the byte range specified by buffer.
 *
 * @c buffer can be a pointer to a character buffer or an output iterator. If the engine is in Unicode mode then the
 * codepoint will be encoded using UTF-8. Otherwise the codepoint will be truncated to fit into the ASCII encoding.
 *
 * @tparam octet_iterator The type of the output sequence.
 * @param cp The codepoint to encode
 * @param buffer The buffer to write the encoded data to.
 * @return The value of the iterator after appending all bytes to the sequence.
 */
template<typename octet_iterator>
octet_iterator encode(codepoint_t cp, octet_iterator buffer) {
	if (Unicode_text_mode) {
		try {
			return utf8::append(cp, buffer);
		} catch(const std::exception& e) {
			Error(LOCATION, "Exception while encoding Unicode code point %" PRIu32 ": %s", (uint32_t)cp, e.what());
			return buffer;
		}
	} else {
		// In the legacy mode every code point is exactly one char
		*(buffer++) = (char)cp;
		return buffer;
	}
}

/**
 * @brief Counts the number of code points in the specified byte sequence
 *
 * @note This respects the Unicode mod setting and should be used where the text contains Unicode characters.
 *
 * @tparam octet_iterator The type of the byte sequence.
 * @param start An iterator located at the start of the sequence
 * @param end An iterator which signals the end of the sequence
 * @return The number of codepoints found between @c start and @c end
 */
template<typename octet_iterator>
size_t num_codepoints(octet_iterator start, octet_iterator end) {
	if (Unicode_text_mode) {
		try {
			return static_cast<size_t>(utf8::distance(start, end));
		} catch(const std::exception& e) {
			Error(LOCATION, "Exception while counting Unicode code points: %s", e.what());
			return 0;
		}
	} else {
		return static_cast<size_t>(std::distance(start, end));
	}
}

template<typename octet_iterator>
void advance(octet_iterator& start, size_t n, octet_iterator end) {
	if (Unicode_text_mode) {
		utf8::advance(start, n, end);
	} else {
		start = std::min(start + n, end);
	}
}

enum class Encoding { Encoding_current, Encoding_utf8, Encoding_iso8859_1 };

bool string_is_ascii_only(const char* str, size_t len);
const char* get_encoding_string(Encoding encoding);
bool convert_encoding(SCP_string& buffer, const char* src, Encoding encoding_src, Encoding encoding_dest = Encoding::Encoding_current);
}

//
//

#include "unicode.h"

namespace unicode {

text_iterator::text_iterator(const char* in_current_byte, const char* in_range_start_byte, const char* in_range_end_byte) :
	current_byte(in_current_byte), range_end_byte(in_range_end_byte), range_start_byte(in_range_start_byte) {
	if (range_end_byte == nullptr) {
		range_end_byte = current_byte + strlen(current_byte);
	}
}
text_iterator& unicode::text_iterator::operator++() {
	if (Unicode_text_mode) {
		try {
			// Increment by UTF-8 encoded codepoints
			utf8::next(current_byte, range_end_byte);
		} catch(const std::exception& e) {
			Error(LOCATION, "Exception while incrementing UTF-8 sequence near '%.16s': %s", current_byte, e.what());
			return *this;
		}
	} else {
		// Increment by byte
		++current_byte;
	}

	return *this;
}
text_iterator& text_iterator::operator--() {
	if (Unicode_text_mode) {
		try {
			// Decrement by UTF-8 encoded codepoints
			utf8::prior(current_byte, range_start_byte);
		} catch(const std::exception& e) {
			Error(LOCATION, "Exception while decrementing text iterator near '%.16s': %s", current_byte, e.what());
			return *this;
		}
	} else {
		// Increment by byte
		--current_byte;
	}

	return *this;
}
text_iterator::value_type text_iterator::operator*() {
	if (Unicode_text_mode) {
		try {
			return utf8::peek_next(current_byte, range_end_byte);
		} catch(const std::exception& e) {
			Error(LOCATION, "Exception while decoding UTF-8 sequence near '%.16s': %s", current_byte, e.what());
			return 0;
		}
	} else {
		// Use the unsigned byte value here to avoid integer overflows
		return (codepoint_t)*reinterpret_cast<const uint8_t*>(current_byte);
	}
}
const char* text_iterator::pos() const {
	return current_byte;
}
bool text_iterator::operator==(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return current_byte == rhs.current_byte;
}
bool text_iterator::operator!=(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return !(rhs == *this);
}
bool text_iterator::operator<(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return current_byte < rhs.current_byte;
}
bool text_iterator::operator>(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return rhs < *this;
}
bool text_iterator::operator<=(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return !(rhs < *this);
}
bool text_iterator::operator>=(const text_iterator& rhs) const {
	Assertion(is_from_same_range(rhs), "Iterators must be from the same byte range!");

	return !(*this < rhs);
}
text_iterator text_iterator::operator+(ptrdiff_t diff) const {
	return text_iterator(current_byte + diff, range_start_byte, range_end_byte);
}
text_iterator text_iterator::operator-(ptrdiff_t diff) const {
	return text_iterator(current_byte - diff, range_start_byte, range_end_byte);
}
bool text_iterator::is_from_same_range(const text_iterator& other) const {
	return range_start_byte == other.range_start_byte && range_end_byte == other.range_end_byte;
}

codepoint_range::codepoint_range(const char* in_start, const char* in_end) : start(in_start), end_ptr(in_end) {
	Assertion(start != nullptr, "Start of the string must be a valid pointer!");

	if (end_ptr == nullptr) {
		// Automatically determine the end of the string
		end_ptr = start + strlen(start);
	}
}
text_iterator codepoint_range::begin() {
	return text_iterator(start, start, end_ptr);
}
text_iterator codepoint_range::end() {
	return text_iterator(end_ptr, start, end_ptr);
}

size_t encoded_size(codepoint_t cp) {
	if (Unicode_text_mode) {
		try {
			return utf8::encoded_width(cp);
		} catch(const std::exception& e) {
			Error(LOCATION,
				  "Exception while computing encoded size of Unicode code point %" PRIu32 ": %s",
				  (uint32_t) cp,
				  e.what());
			return 1;
		}
	} else {
		// In the legacy mode every code point is exactly one char
		return 1;
	}
}
}

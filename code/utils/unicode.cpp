//
//

#include "unicode.h"

namespace unicode {

text_iterator::text_iterator(const char* in_current_byte, const char* in_range_start_byte, const char* in_range_end_byte) :
	current_byte(in_current_byte), range_end_byte(in_range_end_byte), range_start_byte(in_range_start_byte) {
	if (range_end_byte == nullptr) {
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
// This suppresses a GCC bug where it thinks that in_current_byte is null in release builds
#pragma GCC diagnostic ignored "-Wnonnull"
#endif
		range_end_byte = in_current_byte + strlen(in_current_byte);
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
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
	if (diff < 0) {
		// Use minus operator to avoid duplicating code
		return operator-(-diff);
	}

	auto iter = *this;
	for (ptrdiff_t i = 0; i < diff; ++i) {
		++iter;
	}
	return iter;
}
text_iterator text_iterator::operator-(ptrdiff_t diff) const {
	if (diff < 0) {
		// Use plus operator to avoid duplicating code
		return operator+(-diff);
	}

	auto iter = *this;
	for (ptrdiff_t i = 0; i < diff; ++i) {
		--iter;
	}
	return iter;
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

bool string_is_ascii_only(const char* str, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (str[i] > 0x79 || str[i] < 0)
			return false;
	}

	return true;
}

const char* get_encoding_string(Encoding encoding) {
	switch (encoding) {
	case Encoding::Encoding_iso8859_1:
		return "ISO-8859-1";
	case Encoding::Encoding_utf8:
		return "UTF-8";
	case Encoding::Encoding_current:
	default:
		UNREACHABLE("Unknown encoding type was passed.\n");
		return "";
	}
}

bool convert_encoding(SCP_string& buffer, const char* src, Encoding encoding_src, Encoding encoding_dest) {

	if (encoding_src == Encoding::Encoding_current)
		encoding_src = Unicode_text_mode ? Encoding::Encoding_utf8 : Encoding::Encoding_iso8859_1;

	if (encoding_dest == Encoding::Encoding_current)
		encoding_dest = Unicode_text_mode ? Encoding::Encoding_utf8 : Encoding::Encoding_iso8859_1;

	//We are already in the correct encoding, the string is correct
	if (encoding_src == encoding_dest) {
		buffer.assign(src);
		return true;
	}

	//If not, convert
	auto len = strlen(src);

	// Validate if no change needs to be done
	if (string_is_ascii_only(src, len))
	{
		// turns out this is valid anyways
		buffer.assign(src);
		return true;
	}

	size_t newlen = len;
	std::unique_ptr<char[]> newstr(new char[newlen]);

	do {
		auto in_str = src;
		auto in_size = len;
		auto out_str = newstr.get();
		auto out_size = newlen;

		auto iconv = SDL_iconv_open(get_encoding_string(encoding_dest), get_encoding_string(encoding_src));
		auto err = SDL_iconv(iconv, &in_str, &in_size, &out_str, &out_size);
		SDL_iconv_close(iconv);

		// SDL returns the number of processed character on success;
		// error codes are (size_t)-1 through -4
		if (err < (size_t)-100)
		{
			// successful re-encoding
			buffer.assign(newstr.get(), newlen - out_size);
			return true;
		}
		else if (err == SDL_ICONV_E2BIG)
		{
			// buffer is not big enough, try again with a bigger buffer. Use a rather conservative size
			// increment since the additional size required is probably pretty small
			newlen += 10;
			newstr.reset(new char[newlen]);
		}
		else
		{
			break;
		}
	} while (true);

	return false;
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command-line parsing functions for z64555's debug console, created for the FreeSpace Source Code project
//
// Portions of this source code are based on works by Volition, Inc. circa 1999. You may not sell or otherwise
// commercially exploit the source or things you created based on the source.
//
// This file contains documentation on locally referenced functions. For documentation on globally referenced
// functions, see consoleparse.h
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////
// @todo Make a fast version of the parse_long, parse_ulong, etc. That just checks the first 1-3 characters. The fast version will be used in retail builds while the slow/safe version will be in debug's
// @todo Maybe make parser functions case-insensitive
////////////////

#include "debugconsole/consoleparse.h"
#include "parse/parselo.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <limits.h>


// ========================= LOCALS =========================
char Command_string[MAX_CLI_LEN];   //!< Command string buffer.
char *Cp = NULL;    //!< Pointer to the commant string

enum state_int {
	si_start   = 0,
	si_end     = 1,
	si_invalid,
	si_sign,        //!< Sign character, '-' '+'
	si_prefix,      //!< prefix character sequence, 0b, 0o, or 0x
	si_numeral,     //!< Numeral state, 0 - 9
	si_numeral_bin,     //!< Numeral altstate, 0, 1
	si_numeral_octal,   //!< Numeral altstate, 0 - 7
	si_numeral_hex      //!< Numeral altstate, 0 - 9 and 'a' - 'f'
};

enum state_float {
	sf_start = 0,
	sf_end = 1,
	sf_invalid,
	sf_sign,		//!< Sign character for mantessa
	sf_whole,		//!< Whole value numeral
	sf_decimal,		//!< Decimal character
	sf_fraction,	//!< Fractional value numeral
	sf_expprefix,	//!< Exponent prefix, 'e', 'E'
	sf_expsign,		//!< Exponent sign
	sf_exponent		//!< Exponent value numeral
};

bool ch_is_sign(char ch);
bool ch_is_numeral(char ch);
bool ch_is_decimal(char ch);
bool ch_is_binary(char ch);
bool ch_is_octal(char ch);
bool ch_is_hex(char ch);

bool ch_is_prefix(char ch);
bool ch_is_binary_prefix(char ch);
bool ch_is_octal_prefix(char ch);
bool ch_is_hex_prefix(char ch);
bool ch_is_exp_prefix(char ch);

/**
 * @brief Returns/Advances past a single token
 *
 * @details Similar in operation to dc_stuff_string_white, but won't throw any error messages and will only grab the
 *   first word. (dc_stuff_string_white may grab quoted strings)
 */
void dc_get_token(SCP_string &out_str);

/**
 * @brief Returns a single token, but does not advances Cp
 *
 * @details Similar in operation to dc_stuff_string_white, but won't throw any error messages and will only grab the
 *   first word. (dc_stuff_string_white may grab quoted strings).
 */
void dc_get_token_no_advance(SCP_string &out_str);

/**
 * @brief Parses a double-precision floating point type. Supports, Whole, Fractional, and Mixed numbers, and supports
 *   scientific notation (exponent prefixed by 'e' or 'E').
 *
 * @param[in] ch   Points to the start of the string to parse
 * @param[in] type The expected type. is thrown along with ch when an unexpected/malformed float is found
 *
 * @return The value of the parsed token
 * @details
 *   The returned double may be cast to a single-precision float, but be sure to check it before doing so!
 */
double dc_parse_double(const char *ch, dc_token type);

/**
 * @brief Parses a long integral type. Supports decimal, binary, octal, and hexidecimal strings.
 *
 * @param[in] ch    Points to the start of the string to parse.
 * @param[in] type  The expected type. Is thrown along with ch when an unexpected/malformed integral is found
 *
 * @details
 * ! Non-decimal values must be prefixed by their corresponding sequence. Binary: "0b", Octal: "0o", Hex: "0x"
 *
 *   The returned long may be cast to a smaller integral, but be sure to check it before doing so!
 *
 *   The only thing left making this function specific to the DC is the expected type. So, if you want to use this
 *   for parsing something other than the debug CL, you'll have to make a set of errParse classes that take a
 *   different expected type.
 */
long   dc_parse_long(const char *ch, dc_token type);

/**
 * @brief Parses an unsigned long integral type. Supports decimal, binary, octal, and hexidecimal strings.
 *
 * @param[in] ch    Points to the start of the string to parse.
 * @param[in] type  The expected type. Is thrown along with ch when an unexpected/malformed integral is found
 *
 * @details
 * ! Non-decimal values must be delimited by their corresponding sequence. Binary: "0b", Octal: "0o", Hex: "0x"
 *   The returned long may be cast to a smaller integral, but be sure to check it before doing so!
 *   The only thing left making this function specific to the DC is the expected type. So, if you want to use this
 *   for parsing something other than the debug CL, you'll have to make a set of errParse classes that take a
 *   different expected type.
 */
ulong  dc_parse_ulong(const char *ch, dc_token type);

// State processes for dc_parse_double
state_float dc_parse_double_sign(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_whole(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_decimal(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_fraction(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_expprefix(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_expsign(const char* &ch_ptr, SCP_string &buffer_str);
state_float dc_parse_double_exponent(const char* &ch_ptr, SCP_string &buffer_str);

// State processes for dc_parse_long and dc_parse_ulong.
state_int dc_parse_long_prefix(const char* &ch_ptr, SCP_string &buffer_str, int &base);
state_int dc_parse_long_sign(const char* &ch_ptr, SCP_string &buffer_str);
state_int dc_parse_long_numeral(const char* &ch_ptr, SCP_string &buffer_str);
state_int dc_parse_long_numeral_bin(const char* &ch_ptr, SCP_string &buffer_str);
state_int dc_parse_long_numeral_hex(const char* &ch_ptr, SCP_string &buffer_str);
state_int dc_parse_long_numeral_octal(const char* &ch_ptr, SCP_string &buffer_str);


// ============================== IMPLEMENTATIONS =============================
inline
bool ch_is_sign(char ch)
{
	return ((ch == '-') || (ch == '+'));
}

inline
bool ch_is_numeral(char ch)
{
	return ((ch >= '0') && (ch <= '9'));
}

inline
bool ch_is_decimal(char ch)
{
	return (ch == '.');
}

inline
bool ch_is_binary(char ch)
{
	return ((ch == '0') || (ch == '1'));
}

inline
bool ch_is_octal(char ch)
{
	return ((ch >= '0') && (ch <= '7'));
}

inline
bool ch_is_hex(char ch)
{
	return (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')));
};

inline
bool ch_is_prefix(char ch)
{
	return (ch == '0');
}

inline
bool ch_is_binary_prefix(char ch)
{
	return ((ch == 'b') || (ch == 'B'));
}

inline
bool ch_is_octal_prefix(char ch)
{
	return ((ch == 'o') || (ch == 'O'));
}

inline
bool ch_is_hex_prefix(char ch)
{
	return ((ch == 'x') || (ch == 'X'));
}

inline
bool ch_is_exp_prefix(char ch)
{
	return ((ch == 'e') || (ch == 'E'));
}

void dc_get_token(SCP_string &out_str) {
	size_t count = 0;
	char *c_ptr;

	Assert(Cp);

	dc_ignore_gray_space();

	out_str = "";
	// Bail if we're at the terminator
	if (*Cp == '\0') {
		return;
	}

	// Scan the string, stopping at grayspace, null terminator, or before we go over MAX_TOKEN_LENGTH
	c_ptr = Cp;
	while (!is_gray_space(*c_ptr) && (*c_ptr != '\0') && (count < MAX_TOKEN_LENGTH)) {
		count++;
		c_ptr++;
	}

	// Copy string into out_str
	out_str.assign(Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;
}

void dc_get_token_no_advance(SCP_string &out_str) {
	size_t count = 0;
	char *c_ptr;

	Assert(Cp);

	dc_ignore_gray_space();

	out_str = "";
	// Bail if we're at the terminator
	if (*Cp == '\0') {
		return;
	}

	// Scan the string, stopping at grayspace, null terminator, or before we go over MAX_TOKEN_LENGTH
	c_ptr = Cp;
	while (!is_gray_space(*c_ptr) && (*c_ptr != '\0') && (count < MAX_TOKEN_LENGTH)) {
		count++;
		c_ptr++;
	}

	// Copy string into out_str
	out_str.assign(Cp, count);
}

void dc_ignore_white_space(void) {
	while (is_white_space(*Cp) && (*Cp != '\0')) {
		Cp++;
	}
}

void dc_ignore_gray_space(void) {
	while (is_gray_space(*Cp) && (*Cp != '\0')) {
		Cp++;
	}
}

bool dc_maybe_stuff_float(float *f)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_float(f);
		return true;
	
	} else {
		*f = 0;
		return false;
	}
}

bool dc_maybe_stuff_int(int *i)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_int(i);
		return true;
	} else {
		*i = 0;
		return false;
	}
}

bool dc_maybe_stuff_uint(uint *i)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_uint(i);
		return true;
	} else {
		*i = 0;
		return false;
	}
}

bool dc_maybe_stuff_ubyte(ubyte *i)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_ubyte(i);
		return true;
	} else {
		*i = 0;
		return false;
	}
}

bool dc_maybe_stuff_boolean(bool *b)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_boolean(b);
		return true;
	} else {
		*b = false;
		return false;
	}
}

bool dc_maybe_stuff_boolean(int *i)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_boolean(i);
		return true;
	} else {
		*i = 0;
		return false;
	}
}

bool dc_maybe_stuff_string(char *out_str, size_t maxlen)
{
	size_t count = 0;
	char *c_ptr = Cp;

	Assert(Cp);
	Assert(out_str);

	// Advance past grayspace, stopping at null terminator
	while (is_gray_space(*c_ptr) && (*c_ptr != '\0')) {
		c_ptr++;
	}

	// Bail if we're at the terminator
	if (*c_ptr == '\0') {
		return false;
	}

	// Scan the string, stopping at null terminator, or before we overflow
	while ((*c_ptr != '\0') && (count < maxlen)) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if (count == maxlen) {
		throw errParse("", DCT_STRING);
	}

	// Copy string into out_str
	strncpy(out_str, Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;

	return true;
}

bool dc_maybe_stuff_string(SCP_string &out_str)
{
	size_t count = 0;
	char *c_ptr = Cp;

	Assert(Cp);

	// Advance past grayspace, stopping at null terminator
	while (is_gray_space(*c_ptr) && (*c_ptr != '\0')) {
		c_ptr++;
	}

	// Bail if we're at the terminator
	if (*c_ptr == '\0') {
		return false;
	}

	// Scan the string, stopping at null terminator, or before we overflow
	while ((*c_ptr != '\0') && (count < out_str.max_size())) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if (count == out_str.max_size()) {
		return false;
	}

	// Copy string into out_str
	out_str.assign(Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;

	return true;
}

bool dc_maybe_stuff_string_white(char *str, size_t len)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_string_white(str, len);
		return true;
	} else {
		*str = '\0';
		return false;
	}
}

bool dc_maybe_stuff_string_white(SCP_string &str)
{
	dc_ignore_gray_space();

	if (*Cp != '\0') {
		dc_stuff_string_white(str);
		return true;
	} else {
		str = "";
		return false;
	}
}

void dc_required_string(char *pstr)
{
	char *str_found = NULL;

	dc_ignore_gray_space();

	if (strncmp(pstr, Cp, strlen(pstr)) == 0) {
		str_found = pstr;
	}

	if (str_found != NULL) {
		// Found a required string
		if (Dc_debug_on) {
			dc_printf("<debug> Found required string [%s]\n", str_found);
		}

		Cp += strlen(str_found);
	} else {
		// Didn't find a required string.
		SCP_string token;
		dc_get_token_no_advance(token);
		throw errParseString(token.c_str(), pstr);
	}
}

int dc_required_string_either(char *str1, char *str2)
{
	char *str_found = NULL;
	int i = -1;

	dc_ignore_gray_space();

	if (strncmp(str1, Cp, strlen(str1)) == 0) {
		str_found = str1;
		i = 0;
	} else if (strncmp(str2, Cp, strlen(str2)) == 0) {
		str_found = str2;
		i = 1;
	}

	if (i > -1) {
		// Found a required string
		if (Dc_debug_on) {
			dc_printf("<debug> Found required string [%s]\n", str_found);
		}

		Cp += strlen(str_found);
	} else {
		// Didn't find a required string.
		SCP_string token;
		dc_get_token_no_advance(token);
		throw errParseString(token.c_str(), str1, str2);
	}

	return i;
}

uint dc_required_string_any(const uint n, ...)
{
	va_list vl;
	SCP_vector<SCP_string> strings;
	const char *str_found = NULL;
	uint i;

	dc_ignore_gray_space();

	// Populate the vector.
	va_start(vl, n);
	for (i = 0; i < n; ++i) {
		strings.push_back(va_arg(vl, char *));
	}
	va_end(vl);

	// Search for the required string. If found, i = index of the string passed
	for (i = 0; i < n; ++i) {
		if (strcmp(Cp, strings[i].c_str()) == 0)
		{
			str_found = strings[i].c_str();
			break;
		}
	}

	if (str_found != NULL) {
		// Found a required string
		if (Dc_debug_on) {
			dc_printf("<debug> Found required string [%s}\n", str_found);
		}

		Cp += strlen(str_found);
	} else {
		// Didn't find a required string.
		SCP_string token;
		dc_get_token_no_advance(token);
		throw errParseString(token.c_str(), strings);
	}

	return i;
}

bool dc_optional_string(const char *pstr)
{
	dc_ignore_gray_space();

	if (strncmp(pstr, Cp, strlen(pstr)) != 0) {
		return false;
	} // Else, optional string was found

	if (Dc_debug_on) {
		dc_printf("<debug> Found optional string [%s]\n", pstr);
	}

	Cp += strlen(pstr);
	return true;
}

bool dc_optional_string_either(const char *str1, const char *str2)
{
	const char *str_found = NULL;

	dc_ignore_gray_space();

	if (strncmp(str1, Cp, strlen(str1)) == 0) {
		str_found = str1;
	} else if (strncmp(str2, Cp, strlen(str2)) == 0) {
		str_found = str2;
	} else {
		return false;
	}

	if (Dc_debug_on) {
		dc_printf("<debug> Found optional string [%s]\n",str_found);
	}

	Cp += strlen(str_found);
	return true;;
}

void dc_parse_init(SCP_string &str)
{
	strcpy_s(Command_string, str.c_str());
	Cp = Command_string;
}

double dc_parse_double(const char *ch, dc_token type) {
	const char *ch_ptr = ch;
	char *end_ptr;
	double ret;
	state_float state = sf_start;
	SCP_string buffer_str;

	while ((*ch_ptr != '\0') && is_white_space(*ch_ptr)) {
		ch_ptr++;
	}

	if (*ch_ptr == '\0') {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] no argument found\n");
		}
		throw errParse("", type);
	}

	do {
		switch (state) {
		case sf_start:
			if (ch_is_sign(*ch_ptr)) {
				state = sf_sign;
				if (*ch_ptr == '-') {
					buffer_str.push_back(*ch_ptr);
				} // Else, sign is positive, and isn't needed on the buffer
				ch_ptr++;

			} else if (ch_is_numeral(*ch_ptr)) {
				state = sf_whole;
				buffer_str.push_back(*ch_ptr);
				ch_ptr++;

			} else if (ch_is_decimal(*ch_ptr)) {
				state = sf_decimal;
				buffer_str.push_back(*ch_ptr);
				ch_ptr++;

			} else {
				if (Dc_debug_on) {
					dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_start\n", *ch_ptr);
				}
				state = sf_invalid;

			}
			break;
		case sf_end:
			if (buffer_str == "") {
				state = sf_invalid;
			} // Else, we can convert the token.
			// Do nothing, and allow the while loop exit condition to trigger
			break;
		case sf_sign:
			state = dc_parse_double_sign(ch_ptr, buffer_str);
			break;

		case sf_whole:
			state = dc_parse_double_whole(ch_ptr, buffer_str);
			break;

		case sf_decimal:
			state = dc_parse_double_decimal(ch_ptr, buffer_str);
			break;

		case sf_fraction:
			state = dc_parse_double_fraction(ch_ptr, buffer_str);
			break;

		case sf_expprefix:
			state = dc_parse_double_expprefix(ch_ptr, buffer_str);
			break;

		case sf_expsign:
			state = dc_parse_double_expsign(ch_ptr, buffer_str);
			break;

		case sf_exponent:
			state = dc_parse_double_exponent(ch_ptr, buffer_str);
			break;

		case sf_invalid:
		default:
			throw errParse(ch, type);
		}
	} while (state != sf_end);

	ret = strtod(buffer_str.c_str(), &end_ptr);

	return ret;
}

state_float dc_parse_double_sign(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_numeral(*ch_ptr)) {
		state = sf_whole;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_decimal(*ch_ptr)) {
		state = sf_decimal;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_sign\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_whole(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_numeral(*ch_ptr)) {
		state = sf_whole;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_decimal(*ch_ptr)) {
		state = sf_decimal;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_exp_prefix(*ch_ptr)) {
		state = sf_expprefix;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = sf_end;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_whole\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_decimal(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_numeral(*ch_ptr)) {
		state = sf_fraction;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_exp_prefix(*ch_ptr)) {
		state = sf_expprefix;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = sf_end;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_decimal\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_fraction(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_numeral(*ch_ptr)) {
		state = sf_fraction;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_exp_prefix(*ch_ptr)) {
		state = sf_expprefix;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = sf_end;
	
	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_faction\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_expprefix(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_sign(*ch_ptr)) {
		state = sf_expsign;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_numeral(*ch_ptr)) {
		state = sf_exponent;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_expprefix\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_expsign(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;

	if (ch_is_numeral(*ch_ptr)) {
		state = sf_exponent;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_expsign\n", *ch_ptr);
		}
	}

	return state;
}

state_float dc_parse_double_exponent(const char* &ch_ptr, SCP_string &buffer_str)
{
	state_float state = sf_invalid;
	
	if (ch_is_numeral(*ch_ptr)) {
		state = sf_exponent;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = sf_end;

	} else {
		state = sf_invalid;
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_double] Invalid character '%c' found in sf_exponent\n", *ch_ptr);
		}
	}

	return state;
}

long dc_parse_long(const char *ch, dc_token type) {
	const char *ch_ptr = ch;
	char *end_ptr = NULL;
	int base = 10;
	long ret;
	state_int state = si_start;
	SCP_string buffer_str;

	while ((*ch_ptr != '\0') && is_white_space(*ch_ptr)) {
		ch_ptr++;
	}

	if (*ch_ptr == '\0') {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] no argument found\n");
		}
		throw errParse("", type);
	}

	do {
		switch (state) {
		case si_start:
			if (ch_is_sign(*ch_ptr)) {
				state = si_sign;
				if (*ch_ptr == '-') {
					buffer_str.push_back(*ch_ptr);
				} // Else, it's positive. Positive sign isn't needed for conversion
				ch_ptr++;

			} else if (ch_is_prefix(*ch_ptr)) {
				// prefixes must be checked before numeral, because they all start with a numeral zero
				state = si_prefix;
				ch_ptr++;
		
			} else if (ch_is_numeral(*ch_ptr)) {
				state = si_numeral;
				buffer_str.push_back(*ch_ptr);
				ch_ptr++;
		
			} else {
				if (Dc_debug_on) {
					dc_printf("<debug> [parse_long] Invalid character '%c' found in si_start\n", *ch_ptr);
				}
				state = si_invalid;

			}
			break;
		case si_end:
			if (buffer_str == "") {
				state = si_invalid;
			} // Else, we can convert the token.
			// Do nothing, and allow the while loop exit condition to trigger
			break;

		case si_sign:
			state = dc_parse_long_sign(ch_ptr, buffer_str);
			break;

		case si_prefix:
			state = dc_parse_long_prefix(ch_ptr, buffer_str, base);
			break;

		case si_numeral_bin:
			state = dc_parse_long_numeral_bin(ch_ptr, buffer_str);
			break;

		case si_numeral_octal:
			state = dc_parse_long_numeral_octal(ch_ptr, buffer_str);
			break;

		case si_numeral_hex:
			state = dc_parse_long_numeral_hex(ch_ptr, buffer_str);
			break;

		case si_numeral:
			state = dc_parse_long_numeral(ch_ptr, buffer_str);
			break;

		case si_invalid:
		default:
			throw errParse(ch, type);
			break;
		}
	} while (state != si_end);

	ret = strtol(buffer_str.c_str(), &end_ptr, base);

	// This last check can be omitted once I can verify the operation of strtol in this sense
	if (*end_ptr != '\0') {
		dc_printf("Error: Could not convert all of the buffer '%s'.\n", buffer_str.c_str());
		if (Dc_debug_on) {
			dc_printf("<debug> Buffer value: %s\n", buffer_str.c_str());
			dc_printf("<debug> Return value: %i", ret);
		}
		throw errParse(ch, type);
	}

	return ret;
}

ulong dc_parse_ulong(const char *ch, dc_token type) {
	const char *ch_ptr = ch;
	char *end_ptr;
	int base = 10;
	ulong ret;
	state_int state = si_start;
	SCP_string buffer_str;

	while ((*ch_ptr != '\0') && is_white_space(*ch_ptr)) {
		ch_ptr++;
	}

	if (*ch_ptr == '\0') {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] no argument found\n");
		}
		throw errParse("", type);
	}

	do {
		switch (state) {
		case si_start:
			if (ch_is_prefix(*ch_ptr)) {
				// prefixes must be checked before numeral, because they all start with a numeral zero
				state = si_prefix;
				ch_ptr++;
		
			} else if (ch_is_numeral(*ch_ptr)) {
				state = si_numeral;
				buffer_str.push_back(*ch_ptr);
				ch_ptr++;
		
			} else {
				if (Dc_debug_on) {
					dc_printf("<debug> [parse_long] Invalid character '%c' found in si_start\n", *ch_ptr);
				}
				state = si_invalid;

			}
			break;
		case si_end:
			if (buffer_str == "")
			{
				state = si_invalid;
			} // Else, we can convert the token.
			// Do nothing, and allow the while loop exit condition to trigger
			break;

		case si_prefix:
			state = dc_parse_long_prefix(ch_ptr, buffer_str, base);
			break;

		case si_numeral_bin:
			state = dc_parse_long_numeral_bin(ch_ptr, buffer_str);
			break;

		case si_numeral_octal:
			state = dc_parse_long_numeral_octal(ch_ptr, buffer_str);
			break;

		case si_numeral_hex:
			state = dc_parse_long_numeral_hex(ch_ptr, buffer_str);
			break;

		case si_numeral:
			state = dc_parse_long_numeral(ch_ptr, buffer_str);
			break;

		case si_invalid:
		default:
			throw errParse(ch, type);
		}
	} while (state != si_end);

	ret = strtoul(buffer_str.c_str(), &end_ptr, base);

	// This last check can be omitted once I can verify the operation of strtol in this sense
	if (end_ptr != ch_ptr) {
		dc_printf("Error: Could not convert all of the buffer '%s'.\n", buffer_str.c_str());
		if (Dc_debug_on) {
			dc_printf("<debug> Buffer value: %s\n", buffer_str.c_str());
			dc_printf("<debug> Return value: %i", ret);
		}
		throw errParse(ch, type);
	}

	return ret;
}

void dc_stuff_float(float *f)
{
	double value_d;
	SCP_string token;

	dc_ignore_gray_space();

	dc_get_token(token);	// Grab the token
	value_d = dc_parse_double(token.c_str(), DCT_FLOAT);	// Parse and convert
	
	// Stuff value if within bounds
	if ((std::abs(value_d) < FLT_MAX) && (std::abs(value_d) > FLT_MIN)) {
		*f = static_cast<float>(value_d);
	} else {
		throw errParse(token.c_str(), DCT_FLOAT);
	}
}

void dc_stuff_int(int *i)
{
	long value_l;
	SCP_string token;

	dc_ignore_gray_space();

	dc_get_token(token);
	value_l = dc_parse_long(token.c_str(), DCT_INT);

	if ((value_l < INT_MAX) && (value_l > INT_MIN)) {
		*i = value_l;

	} else {
		throw errParse(token.c_str(), DCT_INT);
	}
}

void dc_stuff_uint(uint *i)
{
	ulong value_l;
	SCP_string token;

	dc_ignore_gray_space();

	dc_get_token(token);
	value_l = dc_parse_long(Cp, DCT_INT);

	if (value_l < UINT_MAX) {
		*i = value_l;

	} else {
		throw errParse(token.c_str(), DCT_INT);
	}
}

void dc_stuff_ubyte(ubyte *i)
{
	ulong value_ul;
	SCP_string token;

	dc_ignore_gray_space();

	dc_get_token(token);
	value_ul = dc_parse_ulong(Cp, DCT_UBYTE);

	// Since some system's chars may be greater than 1 byte, we can't use UCHAR_MAX for a UBYTE
	if (value_ul <= 255) {
		*i = static_cast<ubyte>(value_ul);

	} else {
		throw errParse(token.c_str(), DCT_UBYTE);
	}
}

void dc_stuff_boolean(bool *b)
{
	SCP_string token;

	dc_get_token(token);

	if ((token == "yes")
		|| (token == "true")
		|| (token == "ja")          // German
		|| (token == "Oui")         // French
		|| (token == "si")          // Spanish
//		|| (token == "ita vero")    // Latin, not supported
		|| (token == "HIja") || (token == "HISLaH")     // Klingon
		|| (token == "1"))
	{
		*b = true;

	} else if ((token == "no")
		|| (token == "false")
		|| (token == "nein")    // German
		|| (token == "Non")     // French
//		|| (token == "no")      // Spanish, redundant with English "no"
//		|| (token == "minime")  // Latin, not supported
		|| (token == "ghobe'")  // Klingon
		|| (token == "0"))
	{
		*b = false;

	} else {
		throw errParse(token.c_str(), DCT_BOOL);
	}
}

void dc_stuff_boolean(int *i)
{
	bool value_b;

	dc_stuff_boolean(&value_b);

	value_b ? *i = 1 : *i = 0;
}

void dc_stuff_string(char *out_str, size_t maxlen = MAX_TOKEN_LENGTH)
{
	size_t count = 0;
	char *c_ptr = Cp;
	SCP_string token;

	Assert(Cp);
	Assert(out_str);

	dc_ignore_gray_space();

	// Bail if we're at the terminator
	if (*Cp == '\0') {
		throw errParse("Nothing!", DCT_STRING);
	}

	// Scan the string, stopping at null terminator, or before we overflow
	c_ptr = Cp;
	while ((*c_ptr != '\0') && (count < maxlen)) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if (count == maxlen) {
		token.assign(Cp, maxlen);
		throw errParseOverflow(token.c_str(), maxlen);
	}

	// Copy string into out_str
	strncpy(out_str, Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;
}

void dc_stuff_string(SCP_string &out_str)
{
	size_t count = 0;
	char *c_ptr = Cp;

	Assert(Cp);

	dc_ignore_gray_space();

	// Bail if we're at the terminator
	if (*Cp == '\0') {
		throw errParse("Nothing!", DCT_STRING);
	}

	// Scan the string, stopping at null terminator, or before we overflow
	c_ptr = Cp;
	while ((*c_ptr != '\0') && (count < out_str.max_size())) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if ((count == out_str.max_size()) && (*c_ptr != '\0')) {
		throw errParse("SCP_string overflow!", DCT_STRING);
	}

	// Copy string into out_str
	out_str.assign(Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;
}

void dc_stuff_string_white(char *out_str, size_t maxlen)
{
	size_t count = 0;
	char *c_ptr = Cp;

	Assert(Cp);
	Assert(out_str);

	dc_ignore_gray_space();

	// Bail if we're at the terminator
	if (*Cp == '\0') {
		throw errParse("Nothing!", DCT_STRING);
	}

	// Scan the string, stopping at grayspace, null terminator, or before we overflow
	c_ptr = Cp;
	while (!is_gray_space(*c_ptr) && (*c_ptr != '\0') && (count < maxlen)) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if (count == maxlen) {
		throw errParse("", DCT_STRING);
	}

	// Copy string into out_str
	strncpy(out_str, Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;
}

void dc_stuff_string_white(SCP_string &out_str)
{
	size_t count = 0;
	char *c_ptr;

	Assert(Cp);

	dc_ignore_gray_space();

	// Bail if we're at the terminator
	if (*Cp == '\0') {
		throw errParse("Nothing!", DCT_STRING);
	}

	// Scan the string, stopping at grayspace, null terminator, or before we overflow
	c_ptr = Cp;
	while (!is_gray_space(*c_ptr) && (*c_ptr != '\0') && (count < out_str.max_size())) {
		count++;
		c_ptr++;
	}

	// Bail if overflow
	if (count == out_str.max_size()) {
		throw errParse("", DCT_STRING);
	}

	// Copy string into out_str
	out_str.assign(Cp, count);
	
	// Advance the parser pointer past what we copied
	Cp = c_ptr;
}

inline
state_int dc_parse_long_prefix(const char* &ch_ptr, SCP_string &buffer_str, int &base) {
	state_int state = si_invalid;

	if (ch_is_binary_prefix(*ch_ptr)) {
		state = si_numeral_bin;
		base = 2;
		ch_ptr++;
	
	} else if (ch_is_octal_prefix(*ch_ptr)) {
		state = si_numeral_octal;
		base = 8;
		ch_ptr++;
	
	} else if (ch_is_hex_prefix(*ch_ptr)) {
		state = si_numeral_hex;
		base = 16;
		ch_ptr++;

	} else if (ch_is_numeral(*ch_ptr)) {
		// User passed something like 0123
		// Just ignore the first 0, if the user passed something like 00001 then state si_numeral can handle it
		state = si_numeral;
		ch_ptr++;
	
	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		// Just a 0 was passed.
		buffer_str.push_back('0');
		state = si_end;

	} else {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_prefix\n", *ch_ptr);
		}
		state = si_invalid;

	}
	return state;
}

inline
state_int dc_parse_long_sign(const char* &ch_ptr, SCP_string &buffer_str) {
	state_int state;
	if (ch_is_numeral(*ch_ptr)) {
		state = si_numeral;
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;

	} else if (ch_is_prefix(*ch_ptr)) {
		state = si_prefix;
		ch_ptr++;

	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		// Error, no value found
		state = si_invalid;

	} else {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_sign\n", *ch_ptr);
		}
		state = si_invalid;

	}
	return state;
}

inline
state_int dc_parse_long_numeral(const char* &ch_ptr, SCP_string &buffer_str) {
	state_int state = si_numeral;

	if (ch_is_numeral(*ch_ptr)) {
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;
	
	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = si_end;
	
	} else {
		// Invalid character, throw and bail
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_numeral\n", *ch_ptr);
		}
		state = si_invalid;

	}
	return state;
}

inline
state_int dc_parse_long_numeral_bin(const char* &ch_ptr, SCP_string &buffer_str) {
	state_int state = si_numeral_bin;

	if (ch_is_binary(*ch_ptr)) {
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;
	
	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = si_end;
	
	} else {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_numeral_bin\n", *ch_ptr);
		}
		state = si_invalid;

	}
	return state;
}

inline
state_int dc_parse_long_numeral_hex(const char* &ch_ptr, SCP_string &buffer_str) {
	state_int state = si_numeral_hex;

	if (ch_is_hex(*ch_ptr)) {
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;
	
	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = si_end;
	
	} else {
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_numeral_hex\n", *ch_ptr);
		}
		state = si_invalid;
	}
	return state;
}

inline
state_int dc_parse_long_numeral_octal(const char* &ch_ptr, SCP_string &buffer_str) {
	state_int state = si_numeral_octal;

	if (ch_is_octal(*ch_ptr)) {
		buffer_str.push_back(*ch_ptr);
		ch_ptr++;
	
	} else if ((*ch_ptr == '\0') || is_white_space(*ch_ptr)) {
		state = si_end;
	
	} else {
		// Invalid character, throw and bail
		if (Dc_debug_on) {
			dc_printf("<debug> [parse_long] Invalid character '%c' found in si_numeral_octal\n", *ch_ptr);
		}
		state = si_invalid;
	}
	return state;
}

#ifndef _CONSOLEPARSE_H
#define _CONSOLEPARSE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command-line parsing functions for z64555's debug console, created for the FreeSpace Source Code project
//
// Portions of this source code are based on works by Volition, Inc. circa 1999. You may not sell or otherwise
// commercially exploit the source or things you created based on the source.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @file consoleparse.h
 * @brief Parsing functions for the command line. Previously known as the command line scanner
 *
 * @details A lot of functions here are blatently copied from parselo.h :D
  */

#include "debugconsole/console.h"
#include "globalincs/pstypes.h"

#include <cstdarg>

#define MAX_CLI_LEN 512
#define MAX_TOKEN_LENGTH 255

enum dc_token {
	DCT_NONE = 0,   //!< No token
	DCT_STRING,     //!< String
	DCT_FLOAT,      //!< Floating point
	DCT_INT,        //!< Integral
	DCT_UINT,       //!< Unsigned Integral
	DCT_BYTE,       //!< Integral with values between -128 and 127
	DCT_UBYTE,      //!< Integral with values between 0 and 255
	DCT_BOOL,       //!< Integral or string evaluated as a boolean

	DCT_MAX_ITEMS   //!< Maximum number of dc_token elements. Primarily used as an end value in loops
};

/**
 * @class errParse
 *
 * @brief Class thrown when a required token is not found
 *
 *  @details This is a basic parser error, it contains the token (a single word) that was found, and the expected token
 *      type. Some DCT's, such as the DCT_STRING's, have their own specific requirements for error handling, and as
 *      such get their own class derived from errParse. The catching routines should be able to catch the derived error
 *      objects, but if not, they can be caught by a routine looking for the base class and then be casted to their
 *      proper type.
 */
class errParse {
public:
	SCP_string found_token;
	dc_token expected_type;

	/**
	*  @brief Invalid/Unexpected token constructor
	*  @param [in] found_str The token that was found
	*  @param [in] expected_dct The token type that was expected
	*/
	errParse(const char *found_str, dc_token expected_dct)
		: found_token(found_str), expected_type(expected_dct)
	{
	}
};

/**
 * @class errParseString
 *
 * @brief Class thrown when an expected string was not found. Can/should contain all of the expected strings.
 */
class errParseString : public errParse
{
public:
	SCP_vector<SCP_string> expected_tokens;

	/**
	 *  @brief Invalid/Unexpected token constructor.
	 *  
	 *  @param [in] found_str The string that was found
	 *  @param [in] str The token that was expected
	 */
	errParseString(const char *found_str, char *str)
	    : errParse(found_str, DCT_STRING)
	{
		expected_tokens.push_back(str);
	}

	/**
	 *  @brief Invalid/Unexpected token constructor.
	 *  
	 *  @param [in] found_str The string that was found
	 *  @param [in] str1 The first token that was expected
	 *  @param [in] str2 The second token that was expected
	 */
	errParseString(const char *found_str, char *str1, char *str2)
	    : errParse(found_str, DCT_STRING)
	{
		expected_tokens.push_back(str1);
		expected_tokens.push_back(str2);
	}

	/**
	 *  @brief Invalid/Unexpected token constructor.
	 *  
	 *  @param [in] found_str The string that was found
	 *  @param [in] strings The strings that were expected
	 */
	errParseString(const char *found_str, SCP_vector<SCP_string> &strings)
	    : errParse(found_str, DCT_STRING), expected_tokens(strings)
	{
	}
};

/**
 * @class errParseOverflow
 *
 * @brief Class thrown when the parsed string or token could not be stuffed into the smaller destination container
 *
 * @var found_str Contains the first 'len' valid characters.
 */
class errParseOverflow : public errParse
{
public:
	size_t len;		//!< The size of the destination container

	errParseOverflow(const char *found_str, size_t _len)
		: errParse(found_str, DCT_STRING), len(_len)
	{
	}
};

/**
 * @brief Initializes the DC command line parser
 */
void dc_parse_init(SCP_string &str);

/**
 * @brief Advances the parser past whitespace characters
 */
void dc_ignore_white_space(void);

/**
 * @brief Advances the parser past grayspace characters
 */
void dc_ignore_gray_space(void);

// Required/Optional Token
/**
 * @brief Searches for a specified required string, throwing an errParse if not found.
 *
 * @param[in] pstr The string to search for
 *
 * @throws errParseString with the required string
 */
void dc_required_string(char *pstr);

/**
 * @brief Searchs for either of the specified required strings, throwing an errParse if neither are found
 *
 * @param[in] str1 The first string to search for
 * @param[in] str2 The second string to search for
 *
 * @retval 0 if str1 was found, or
 * @retval 1 if str2 was found
 *
 * @throws errParseString with the required strings
 */
int dc_required_string_either(char *str1, char *str2);

/**
 * @brief Searches for specified required strings
 *
 * @param[in] n The number of char[] or c_str's given
 * @param[in] ... A comma delimited list of one or more required strings.
 *
 * @returns The index of the specified required strings (as if they were an array)
 *
 * @throws errParseString with the required strings
 */
uint dc_required_string_any(const uint n, ...);

/**
 * @brief Searches for an optional string
 *
 * @param[in] pstr The char[] to look for
 *
 * @retval true if the string was found,
 * @retval false otherwise
 */
bool dc_optional_string(const char *pstr);

/**
 * @brief Searches for an optional string and it's alias
 *
 * @param[in] str1 The char[] to look for
 * @param[in] str2 The alternative char[] to look for
 *
 * @retval true if the string was found,
 * @retval false otherwise
 */
bool dc_optional_string_either(const char *str1, const char *str2);


// ==========================
// Stuffers
// ==========================

/**
 * @brief Stuffs a float to the given variable.
 *
 * @param[in] f  The float variable to stuff to
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_float(float *f);

/**
 * @brief Stuffs an int to the given variable. Supports binary (0b), hexadecimal (0x), and octal (0o) formats
 *
 * @param[in] i  The int variable to stuff to
 *
 * @details The binary, hexadecimal, and octal formats must be prefixed by their associated string.
 *   Ex: "0xDEADBEEF" would be parsed properly while "DEADBEEF" would throw an error
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_int(int *i);

/**
 * @brief Stuffs an unsigned int to the given variable. Supports binary (0b), hexadecimal (0x), and octal (0o) formats
 *
 * @param[in] i  The unsigned int variable to stuff to
 *
 * @details The binary, hexadecimal, and octal formats must be prefixed by their associated string.
 *   Ex: "0xDEADBEEF" would be parsed properly while "DEADBEEF" would throw an error
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_uint(uint *i);

/**
 * @brief Stuffs an unsigned byte to the given variable. Supports binary (0b), hexadecimal (0x), and octal (0o) formats
 *
 * @param[in] i  The ubyte variable to stuff to
 *
 * @details The binary, hexadecimal, and octal formats must be prefixed by their associated string.
 *   Ex: "0x0F" would be parsed properly while "0F" would throw an error
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_ubyte(ubyte *i);

/**
 * @brief stuffs a boolean evaluated integer or string into the given variable.
 *
 * @param[in] b The bool variable to stuff to
 *
 * @details Supports a number of literal strings as true and false, including "true", "false", "yes", "no" and the
 *   yes/no equivalents in other languages supported in localization.
 *   TODO: Make a static string map to handle this instead of being hard-coded
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_boolean(bool *b);

/**
 * @brief stuffs a boolean evaluated integer or string into the given variable.
 *
 * @param[in] i The int variable to stuff to. 0 is false, 1 is true
 *
 * @details Supports a number of literal strings as true and false, including "true", "false", "yes", "no" and the
 *   yes/no equivalents in other languages supported in localization.
 *   TODO: Make a static string map to handle this instead of being hard-coded
 *
 * @throws errParse if an unexpected or otherwise malformed float string is found.
 * @throws errParse if nothing was found
 */
void dc_stuff_boolean(int *i);

/**
 * @brief Stuffs a string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string
 * @param[in]  maxlen   Maximum length to copy. (maxlen <= sizeof(str)) && (maxlen <= MAX_TOKEN_LENGTH)
 *
 * @throws errParse when nothing left was found on the command line
 * @throws errParseOverflow when parser cannot stuff the entirety of the found string into out_str
 */
void dc_stuff_string(char *str, size_t maxlen);

/**
 * @brief Stuffs a string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string
 *
 * @throws errParse when nothing left was found on the command line
 */
void dc_stuff_string(SCP_string &str);

/**
 * @brief Stuffs a whitespace delimited string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string
 * @param[in]  len   Maximum length to copy. (len <= sizeof(str)) && (len <= MAX_TOKEN_LENGTH)
 *
 * @throws errParse when nothing left was found on the command line
 * @throws errParseOverflow when parser cannot stuff the entirety of the found string into out_str
 */
void dc_stuff_string_white(char *str, size_t len);

/**
 * @brief Stuffs a whitespace delimited string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string
 *
 * @throws errParse when nothing left was found on the command line
 */
void dc_stuff_string_white(SCP_string &str);

/**
 * @brief Tries to stuff a float from the Command_string.
 *
 * @param[in] f  The float variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *f = 0 and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *f = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_float(float *f);

/**
 * @brief Tries to stuff an int from the Command_string.
 *
 * @param[in] i  The int variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *i = 0 and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *i = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_int(int *i);

/**
 * @brief Tries to stuff an uint from the Command_string.
 *
 * @param[in] i  The uint variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *i = 0 and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *i = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_uint(uint *i);

/**
 * @brief Tries to stuff an ubyte from the Command_string.
 *
 * @param[in] i  The ubyte variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *i = 0 and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *i = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_ubyte(ubyte *i);

/**
 * @brief Tries to stuff a bool from the Command_string.
 *
 * @param[in] b  The bool variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *b = false and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *b = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_boolean(bool *b);

/**
 * @brief Tries to stuff an int with a bool value from the Command_string.
 *
 * @param[in] i  The int variable to maybe stuff.
 *
 * @details
 *   If there's nothing on the command line, *i = 0 and false is returned
 *
 *   If there's something on the command line, and we're able to convert it, *i = the converted value, true is
 *     returned, and the parser is advanced past the token
 *
 *   If there's something on command line, but we can't convert it, an errParse is thrown
 */
bool dc_maybe_stuff_boolean(int *i);

/**
 * @brief Tries to stuff a string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string. If nothing was found, str = ""
 * @param[in]  len   Maximum length to copy. (len <= sizeof(str)) && (len <= MAX_TOKEN_LENGTH)
 *
 * @retval true if the operation was successful,
 * @retval false otherwise
 *
 * @throws errParseOverflow when parser cannot stuff the entirety of the found string into out_str
 */
bool dc_maybe_stuff_string(char *str, size_t len);

/**
 * @brief Tries to stuff a string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string. If nothing was found, str = ""
 *
 * @retval true if the operation was successful,
 * @retval false otherwise
 */
bool dc_maybe_stuff_string(SCP_string &str);

/**
 * @brief Tries to stuff a whitespace delimited string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string. If nothing was found, str = ""
 * @param[in]  len   Maximum length to copy. (maxlen <= sizeof(str)) && (maxlen <= MAX_TOKEN_LENGTH)
 *
 * @retval true if the operation was successful,
 * @retval false otherwise
 *
 * @throws errParseOverflow when parser cannot stuff the entirety of the found string into out_str
 */
bool dc_maybe_stuff_string_white(char *str, size_t len);

/**
 * @brief Tries to stuff a whitespace delimited string to out_str from the command line, stopping at the end of the command line
 *
 * @param[out] str  Destination string. If nothing was found, str = ""
 *
 * @retval true if the operation was successful,
 * @retval false otherwise
 */
bool dc_maybe_stuff_string_white(SCP_string &str);

#endif // _CONSOLEPARSE_H

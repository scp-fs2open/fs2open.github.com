#pragma once

#include "cfile/cfile.h"

namespace util {

enum class Encoding {
	ASCII, UTF8, UTF16LE, UTF16BE, UTF32LE, UTF32BE
};

/**
 * @brief Guesses the encoding of the given content by looking at the first few bytes
 * @param buf The content to check; need not be null-terminated
 * @param len The length of the content in bytes
 * @param assume_utf8 Set the @true true if no BOM should be treated as UTF-8 instead of ASCII
 * @return The guessed encoding of the content
 */
Encoding guess_encoding(const char* buf, size_t len, bool assume_utf8 = true);

/**
 * @brief Gets the length of the Byte Order Mark (BOM), if any, that has to be skipped to get to the real text
 * @param buf The content to check; need not be null-terminated
 * @param len The length of the content in bytes
 * @return The length of the BOM in bytes, or 0 if there is none
 */
size_t bom_length(const char* buf, size_t len);

/**
 * @brief Guesses if the specifies buffer contains Latin1 encoding
 * @param aBuf The buffer to guess
 * @param aLen The length of the buffer
 * @return @c true if the algorithm determined with high probability that the text is Latin1 encoded
 *
 * @note The code of this function was copied from uchardet.
 */
bool guessLatin1Encoding(const char* aBuf, size_t aLen);

/**
 * @brief Checks the encoding of the specified file pointer and possibly skips the BOM if present
 *
 * Use this function if you directly read a text file which may be UTF-8 encoded. This will respect the unicode mode of
 * the current mod so it will also make sure that the file looks like it's ASCII encoded if Unicode mode is disabled.
 *
 * @note If there is a BOM at the start of the file then this function will adjust the read offset of the file pointer
 * to point to the first valid text byte. You can retrieve that offset by using @c start_offset.
 *
 * @param file The file pointer to check
 * @param filename The name of the file. Only used for possible error messages.
 * @param[out] start_offset A pointer to an int variable. If this is a valid pointer then this variable will contain the
 * offset of the first text byte from the start of the file.
 * @param is_lenient For files that can fall back to defaults, such as config files. If true, a UTF-8 file is accepted
 * even when Unicode mode is disabled, and an unsupported encoding produces a warning instead of an error.
 * @return The length of the file in bytes. Does not include the BOM if it exists. If @c is_lenient is true, -1 if the
 * encoding is not supported.
 */
int check_encoding_and_skip_bom(CFILE* file, const char* filename, int* start_offset = nullptr, bool is_lenient = false);
}

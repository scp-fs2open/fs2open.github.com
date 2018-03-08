#pragma once

#include "cfile/cfile.h"

namespace util {

enum class Encoding {
	ASCII, UTF8, UTF16LE, UTF16BE, UTF32LE, UTF32BE
};

/**
 * @brief Guesses the encoding of the given content by looking at the first few bytes
 * @param assume_utf8 Set the @true true if no BOM should be treated as UTF-8 instead of ASCII
 * @return The guessed encoding of the content
 */
Encoding guess_encoding(const SCP_string& content, bool assume_utf8 = true);

/**
 * @brief Determines if the given text has a Byte Order Mark (BOM) that has to be skipped to get to the real text
 * @param content The text to check
 * @return @c true if there is a BOM, @c false otherwise
 */
bool has_bom(const SCP_string& content);

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
 * @return The length of the file in bytes. Does not include the BOM if it exists.
 */
int check_encoding_and_skip_bom(CFILE* file, const char* filename, int* start_offset = nullptr);
}

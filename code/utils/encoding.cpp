//
//

#include "utils/encoding.h"

#include "mod_table/mod_table.h"

namespace util {

Encoding guess_encoding(const SCP_string& content, bool assume_utf8) {
	if (content.size()>= 3 && !strncmp(content.c_str(), "\xEF\xBB\xBF", 3)) {        // UTF-8
		return Encoding::UTF8;
	}

	if (content.size()>= 4 && !strncmp(content.c_str(), "\x00\x00\xFE\xFF", 4)) {    // UTF-32 big-endian
		return Encoding::UTF32BE;
	}

	if (content.size()>= 4 && !strncmp(content.c_str(), "\xFF\xFE\x00\x00", 4)) {    // UTF-32 little-endian
		return Encoding::UTF32LE;
	}

	if (content.size()>= 2 && !strncmp(content.c_str(), "\xFE\xFF", 2)) {            // UTF-16 big-endian
		return Encoding::UTF16BE;
	}

	if (content.size()>= 2 && !strncmp(content.c_str(), "\xFF\xFE", 2)) {            // UTF-16 little-endian
		return Encoding::UTF16LE;
	}

	return assume_utf8 ? Encoding::UTF8 : Encoding::ASCII;
}
bool has_bom(const SCP_string& content) {
	if (content.size()>= 3 && !strncmp(content.c_str(), "\xEF\xBB\xBF", 3)) {        // UTF-8
		return true;
	}

	if (content.size()>= 4 && !strncmp(content.c_str(), "\x00\x00\xFE\xFF", 4)) {    // UTF-32 big-endian
		return true;
	}

	if (content.size()>= 4 && !strncmp(content.c_str(), "\xFF\xFE\x00\x00", 4)) {    // UTF-32 little-endian
		return true;
	}

	if (content.size()>= 2 && !strncmp(content.c_str(), "\xFE\xFF", 2)) {            // UTF-16 big-endian
		return true;
	}

	if (content.size()>= 2 && !strncmp(content.c_str(), "\xFF\xFE", 2)) {            // UTF-16 little-endian
		return true;
	}

	return false;
}
int check_encoding_and_skip_bom(CFILE* file, const char* filename, int* start_offset) {
	cfseek(file, 0, CF_SEEK_SET);

	// Read up to 10 bytes from the file to check if there is a BOM
	SCP_string probe;
	auto probe_size = (size_t)std::min(10, cfilelength(file));
	probe.resize(probe_size);
	cfread(&probe[0], 1, (int) probe_size, file);
	cfseek(file, 0, CF_SEEK_SET);

	auto filelength = cfilelength(file);
	if (start_offset) {
		*start_offset = 0;
	}

	// Determine encoding. Assume UTF-8 if we are in unicode text mode
	auto encoding = util::guess_encoding(probe, Unicode_text_mode);
	if (Unicode_text_mode) {
		if (encoding != util::Encoding::UTF8) {
			//This is probably fatal, so let's abort right here and now.
			Error(LOCATION,
				  "%s is in an Unicode/UTF format that cannot be read by FreeSpace Open. Please convert it to UTF-8\n",
				  filename);
		}
		if (util::has_bom(probe)) {
			// The encoding has to be UTF-8 here so we know that the BOM is 3 Bytes long
			// This makes sure that the first byte we read will be the first actual text byte.
			cfseek(file, 3, SEEK_SET);
			filelength -= 3;

			if (start_offset) {
				*start_offset = 3;
			}
		}
	} else {
		if (encoding != util::Encoding::ASCII) {
			//This is probably fatal, so let's abort right here and now.
			Error(LOCATION,
				  "%s is in Unicode/UTF format and cannot be read by FreeSpace Open without turning on Unicode mode. Please convert it to ASCII/ANSI\n",
				  filename);
		}
	}

	return filelength;
}

// The following code is adapted from the uchardet library, the original licence of the file has been kept

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Universal charset detector code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Shy Shalom <shooshX@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <stdio.h>

#define UDF    0        // undefined
#define OTH    1        //other
#define ASC    2        // ascii capital letter
#define ASS    3        // ascii small letter
#define ACV    4        // accent capital vowel
#define ACO    5        // accent capital other
#define ASV    6        // accent small vowel
#define ASO    7        // accent small other
#define CLASS_NUM   8    // total classes

#define FREQ_CAT_NUM    4

static const unsigned char Latin1_CharToClass[] = { OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 00 - 07
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 08 - 0F
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 10 - 17
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 18 - 1F
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 20 - 27
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 28 - 2F
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 30 - 37
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 38 - 3F
													OTH, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 40 - 47
													ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 48 - 4F
													ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 50 - 57
													ASC, ASC, ASC, OTH, OTH, OTH, OTH, OTH,   // 58 - 5F
													OTH, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 60 - 67
													ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 68 - 6F
													ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 70 - 77
													ASS, ASS, ASS, OTH, OTH, OTH, OTH, OTH,   // 78 - 7F
													OTH, UDF, OTH, ASO, OTH, OTH, OTH, OTH,   // 80 - 87
													OTH, OTH, ACO, OTH, ACO, UDF, ACO, UDF,   // 88 - 8F
													UDF, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 90 - 97
													OTH, OTH, ASO, OTH, ASO, UDF, ASO, ACO,   // 98 - 9F
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // A0 - A7
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // A8 - AF
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // B0 - B7
													OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // B8 - BF
													ACV, ACV, ACV, ACV, ACV, ACV, ACO, ACO,   // C0 - C7
													ACV, ACV, ACV, ACV, ACV, ACV, ACV, ACV,   // C8 - CF
													ACO, ACO, ACV, ACV, ACV, ACV, ACV, OTH,   // D0 - D7
													ACV, ACV, ACV, ACV, ACV, ACO, ACO, ACO,   // D8 - DF
													ASV, ASV, ASV, ASV, ASV, ASV, ASO, ASO,   // E0 - E7
													ASV, ASV, ASV, ASV, ASV, ASV, ASV, ASV,   // E8 - EF
													ASO, ASO, ASV, ASV, ASV, ASV, ASV, OTH,   // F0 - F7
													ASV, ASV, ASV, ASV, ASV, ASO, ASO, ASO,   // F8 - FF
};


/* 0 : illegal
   1 : very unlikely
   2 : normal
   3 : very likely
*/
static const unsigned char Latin1ClassModel[] = {
/*      UDF OTH ASC ASS ACV ACO ASV ASO  */
/*UDF*/  0, 0, 0, 0, 0, 0, 0, 0,
/*OTH*/  0, 3, 3, 3, 3, 3, 3, 3,
/*ASC*/  0, 3, 3, 3, 3, 3, 3, 3,
/*ASS*/  0, 3, 3, 3, 1, 1, 3, 3,
/*ACV*/  0, 3, 3, 3, 1, 2, 1, 2,
/*ACO*/  0, 3, 3, 3, 3, 3, 3, 3,
/*ASV*/  0, 3, 1, 3, 1, 1, 1, 3,
/*ASO*/  0, 3, 1, 3, 1, 1, 3, 3, };

bool guessLatin1Encoding(const char* aBuf, size_t aLen) {
	char mLastCharClass = OTH;
	uint32_t mFreqCounter[FREQ_CAT_NUM];
	for (int i = 0; i < FREQ_CAT_NUM; i++) {
		mFreqCounter[i] = 0;
	}

	unsigned char charClass;
	unsigned char freq;
	for (size_t i = 0; i < aLen; i++) {
		charClass = Latin1_CharToClass[(unsigned char) aBuf[i]];
		freq = Latin1ClassModel[mLastCharClass * CLASS_NUM + charClass];
		if (freq == 0) {
			return false;
		}
		mFreqCounter[freq]++;
		mLastCharClass = charClass;
	}

	float confidence;
	uint32_t total = 0;
	for (int32_t i = 0; i < FREQ_CAT_NUM; i++) {
		total += mFreqCounter[i];
	}

	if (!total) {
		confidence = 0.0f;
	} else {
		confidence = mFreqCounter[3] * 1.0f / total;
		confidence -= mFreqCounter[1] * 20.0f / total;
	}

	if (confidence < 0.0f) {
		confidence = 0.0f;
	}

	return confidence > .5f;
}
}


/*`
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>

#include "parse/parselo.h"
#include "parse/sexp.h"
#include "mission/missionparse.h"
#include "ctype.h"
#include "parse/encrypt.h"
#include "localization/localize.h"
#include "localization/fhash.h"
#include "cfile/cfile.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "globalincs/version.h"



#define	ERROR_LENGTH	64
#define	RS_MAX_TRIES	5

// to know that a modular table is currently being parsed
bool	Parsing_modular_table = false;

char	parse_error_text[128];//for my better error mesages-Bobboau
char	parse_error_text_save[128];

char		Current_filename[128];
char		Current_filename_save[128];
char		Current_filename_sub[128];	//Last attempted file to load, don't know if ex or not.
char		Error_str[ERROR_LENGTH];
int		my_errno;
int		Warning_count, Error_count;
int		Warning_count_save = 0, Error_count_save = 0;
int		fred_parse_flag = 0;
int		Token_found_flag;
jmp_buf	parse_abort;

char 	*Mission_text = NULL;
char	*Mission_text_raw = NULL;
char	*Mp = NULL, *Mp_save = NULL;
char	*token_found;

static int Parsing_paused = 0;

// text allocation stuff
void allocate_mission_text(int size);
static int Mission_text_size = 0;


//	Return true if this character is white space, else false.
int is_white_space(char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == EOLN));
}

// Returns true if this character is gray space, else false (gray space is white space except for EOLN).
int is_gray_space(char ch)
{
	return ((ch == ' ') || (ch == '\t'));
}

int is_parenthesis(char ch)
{
	return ((ch == '(') || (ch == ')'));
}

//	Advance global Mp (mission pointer) past all current white space.
//	Leaves Mp pointing at first non white space character.
void ignore_white_space()
{
	while ((*Mp != EOF_CHAR) && is_white_space(*Mp))
		Mp++;
}

void ignore_gray_space()
{
	while ((*Mp != EOF_CHAR) && is_gray_space(*Mp))
		Mp++;
}

//	Truncate *str, eliminating all trailing white space.
//	Eg: "abc   "   becomes "abc"
//		 "abc abc " becomes "abc abc"
//		 "abc \t"   becomes "abc"
void drop_trailing_white_space(char *str)
{
	int	i = strlen(str) - 1;

	while ((i >= 0) && is_white_space(str[i]))
		i--;

	str[i+1] = 0;
}

//	Ditto for SCP_string
void drop_trailing_white_space(SCP_string &str)
{
	int i = str.length() - 1;

	while ((i >= 0) && is_white_space(str[i]))
		i--;

	str.resize(i+1);
}

//	Eliminate any leading whitespace in str
void drop_leading_white_space(char *str)
{
	int len, first;

	len = strlen(str);
	first = 0;

	// find first non-whitespace
	while ((first < len) && is_white_space(str[first]))
		first++;

	// quick out
	if (first == 0)
		return;

	memmove(str, str+first, len-first);
	str[len-first] = 0;
}

//	Ditto for SCP_string
void drop_leading_white_space(SCP_string &str)
{
	int len, first, i;

	len = str.length();
	first = 0;

	// find first non-whitespace
	while ((first < len) && is_white_space(str[first]))
		first++;

	// quick out
	if (first == 0)
		return;

	// copy chars to beginning of string
	for (i = 0; (first + i) < len; i++)
		str[i] = str[first + i];

	// since i is now off the end of the for loop, it represents the new length
	str.resize(i);
}

// eliminates all leading and trailing white space from a string.  Returns pointer passed in.
char *drop_white_space(char *str)
{
	int s, e;

	s = 0;
	while (str[s] && is_white_space(str[s]))
		s++;

	e = strlen(str) - 1;
	while (e > s) {
		if (!is_white_space(str[e]))
			break;

		e--;
	}

	if (e > s)
		memmove(str, str + s, e - s + 1);

	str[e - s + 1] = 0;
	return str;
}

// ditto for SCP_string
void drop_white_space(SCP_string &str)
{
	int len, newlen, first, last, i;

	len = str.length();
	first = 0;
	last = len - 1;

	// find first non-whitespace
	while ((first < len) && is_white_space(str[first]))
		first++;

	// find last non-whitespace
	while ((last > first) && is_white_space(str[last]))
		last--;

	newlen = last - first + 1;

	// quick out
	if (newlen <= 0)
	{
		str = "";
		return;
	}

	if (first != 0)
	{
		// copy chars to beginning of string
		for (i = 0; i < newlen; i++)
			str[i] = str[first + i];
	}

	str.resize(newlen);
}

//	Advances Mp past current token.
void skip_token()
{
	ignore_white_space();

	while ((*Mp != EOF_CHAR) && !is_white_space(*Mp))
		Mp++;
}

//	Display a diagnostic message if Verbose is set.
//	(Verbose is set if -v command line switch is present.)
void diag_printf(char *format, ...)
{
#ifndef NDEBUG
	char	buffer[8192];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	nprintf(("Parse", "%s", buffer));
#endif
}

//	Grab and return (a pointer to) a bunch of tokens, terminating at
// ERROR_LENGTH chars, or end of line.
char *next_tokens()
{
	int	count = 0;
	char	*pstr = Mp;
	char	ch;

	while (((ch = *pstr++) != EOLN) && (ch != EOF_CHAR) && (count < ERROR_LENGTH-1))
		Error_str[count++] = ch;

	Error_str[count] = 0;
	return Error_str;
}

//	Return the line number given by the current mission pointer, ie Mp.
//	A very slow function (scans all processed text), but who cares how long
//	an error reporting function takes?
int get_line_num()
{
	int	count = 1;
	int	incomment = 0;
	int	multiline = 0;
	char	*stoploc;
	char	*p;

	p = Mission_text;
	stoploc = Mp;

	while (p < stoploc)
	{
		if (*p == EOF_CHAR)
			Assert(0);

		if ( !incomment && (*p == COMMENT_CHAR) )
			incomment = 1;

		if ( !incomment && (*p == '/') && (*(p+1) == '*') ) {
			multiline = 1;
			incomment = 1;
		}

		if ( incomment )
			stoploc++;

		if ( multiline && (*(p-1) == '*') && (*p == '/') ) {
			multiline = 0;
			incomment = 0;
		}

		if (*p++ == EOLN) {
			if ( !multiline && incomment )
				incomment = 0;
			count++;
		}
	}

	return count;
}

//	Call this function to display an error message.
//	error_level == 0 means this is just a warning.
//	!0 means it's an error message.
//	Prints line number and other useful information.
extern int Cmdline_noparseerrors;
void error_display(int error_level, char *format, ...)
{
	char type[8];
	SCP_string error_text;
	va_list args;

	if (error_level == 0) {
		strcpy_s(type, "Warning");
		Warning_count++;
	} else {
		strcpy_s(type, "Error");
		Error_count++;
	}

	va_start(args, format);
	vsprintf(error_text, format, args);
	va_end(args);

	nprintf((type, "%s(line %i): %s: %s\n", Current_filename, get_line_num(), type, error_text.c_str()));

	if(error_level == 0 || Cmdline_noparseerrors)
		Warning(LOCATION, "%s(line %i):\n%s: %s", Current_filename, get_line_num(), type, error_text.c_str());
	else
		Error(LOCATION, "%s(line %i):\n%s: %s", Current_filename, get_line_num(), type, error_text.c_str());
}

//	Advance Mp to the next eoln character.
void advance_to_eoln(char *more_terminators)
{
	char	terminators[128];

	Assert((more_terminators == NULL) || (strlen(more_terminators) < 125));

	terminators[0] = EOLN;
	terminators[1] = (char)EOF_CHAR;
	terminators[2] = 0;
	if (more_terminators != NULL)
		strcat_s(terminators, more_terminators);
	else
		terminators[2] = 0;

	while (strchr(terminators, *Mp) == NULL)
		Mp++;
}

// Advance Mp to the next white space (ignoring white space inside of " marks)
void advance_to_next_white()
{
	int in_quotes = 0;

	while ((*Mp != EOLN) && (*Mp != EOF_CHAR)) {
		if (*Mp == '\"')
			in_quotes = !in_quotes;

		if (!in_quotes && is_white_space(*Mp))
			break;

		if (!in_quotes && is_parenthesis(*Mp))
			break;

		Mp++;
	}
}

// Search for specified string, skipping everything up to that point.  Returns 1 if found,
// 0 if string wasn't found (and hit end of file), or -1 if not found, but end of checking
// block was reached.
int skip_to_string(char *pstr, char *end)
{
	int len, len2 = 0;

	ignore_white_space();
	len = strlen(pstr);
	if (end)
		len2 = strlen(end);

	while ((*Mp != EOF_CHAR) && strnicmp(pstr, Mp, len)) {
		if (end && *Mp == '#')
			return 0;

		if (end && !strnicmp(end, Mp, len2))
			return -1;

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (!Mp || (*Mp == EOF_CHAR))
		return 0;

	Mp += strlen(pstr);
	return 1;
}

// Goober5000
// Advance to start of pstr.  Return 0 is successful, otherwise return !0
int skip_to_start_of_string(char *pstr, char *end)
{
	int len, endlen;

	ignore_white_space();
	len = strlen(pstr);
	if(end)
		endlen = strlen(end);
	else
		endlen = 0;

	while ( (*Mp != EOF_CHAR) && strnicmp(pstr, Mp, len) ) {
		if (end && *Mp == '#')
			return 0;

		if (end && !strnicmp(end, Mp, endlen))
			return 0;

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (!Mp || (*Mp == EOF_CHAR))
		return 0;

	return 1;
}

// Advance to start of either pstr1 or pstr2.  Return 0 is successful, otherwise return !0
int skip_to_start_of_string_either(char *pstr1, char *pstr2, char *end)
{
	int len1, len2, endlen;

	ignore_white_space();
	len1 = strlen(pstr1);
	len2 = strlen(pstr2);
	if(end)
		endlen = strlen(end);
	else
		endlen = 0;

	while ( (*Mp != EOF_CHAR) && strnicmp(pstr1, Mp, len1) && strnicmp(pstr2, Mp, len2) ) {
		if (end && *Mp == '#')
			return 0;

		if (end && !strnicmp(end, Mp, endlen))
			return 0;

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (!Mp || (*Mp == EOF_CHAR))
		return 0;

	return 1;
}

// Find a required string.
// If not found, display an error message, but try up to RS_MAX_TRIES times
// to find the string.  (This is the groundwork for ignoring non-understood
// lines.
//	If unable to find the required string after RS_MAX_TRIES tries, then
//	abort using longjmp to parse_abort.
int required_string(char *pstr)
{
	int	count = 0;

	ignore_white_space();

	while (strnicmp(pstr, Mp, strlen(pstr)) && (count < RS_MAX_TRIES)) {
		error_display(1, "Missing required token: [%s]. Found [%.32s] %s instead.\n", pstr, next_tokens(), parse_error_text);
		advance_to_eoln(NULL);
		ignore_white_space();
		count++;
	}

	if (count == RS_MAX_TRIES) {
		nprintf(("Error", "Error: Unable to find required token [%s] %s\n", pstr, parse_error_text));
		Warning(LOCATION, "Error: Unable to find required token [%s] %s\n", pstr, parse_error_text);
		longjmp(parse_abort, 1);
	}

	Mp += strlen(pstr);
	diag_printf("Found required string [%s]\n", token_found = pstr);
	return 1;
}

int check_for_eof()
{
	ignore_white_space();

	if (*Mp == EOF_CHAR)
		return 1;

	return 0;
}

/**
Returns 1 if it finds a newline character precded by any amount of grayspace.
*/
int check_for_eoln()
{
	ignore_gray_space();

	if(*Mp == EOLN)
		return 1;
	else
		return 0;
}

// similar to optional_string, but just checks if next token is a match.
// It doesn't advance Mp except to skip past white space.
int check_for_string(const char *pstr)
{
	ignore_white_space();

	if (!strnicmp(pstr, Mp, strlen(pstr)))
		return 1;

	return 0;
}

// like check for string, but doesn't skip past any whitespace
int check_for_string_raw(const char *pstr)
{
	if (!strnicmp(pstr, Mp, strlen(pstr)))
		return 1;

	return 0;
}

// Find an optional string.
//	If found, return 1, else return 0.
//	If found, point past string, else don't update pointer.
int optional_string(const char *pstr)
{
	ignore_white_space();
//	mprintf(("lookint for optional string %s",pstr));

	if (!strnicmp(pstr, Mp, strlen(pstr))) {
		Mp += strlen(pstr);
//		mprintf((", found it\n"));
		return 1;
	}
//	mprintf((", didin't find it it\n"));

	return 0;
}

int optional_string_either(char *str1, char *str2)
{
	ignore_white_space();

	if ( !strnicmp(str1, Mp, strlen(str1)) ) {
		Mp += strlen(str1);
		return 0;
	} else if ( !strnicmp(str2, Mp, strlen(str2)) ) {
		Mp += strlen(str2);
		return 1;
	}

	return -1;
}

int required_string_fred(char *pstr, char *end)
{
	char *backup = Mp;

	token_found = pstr;
	if (fred_parse_flag)
		return 0;

	ignore_white_space();
	while (*Mp != EOF_CHAR && strnicmp(pstr, Mp, strlen(pstr))) {
		if ((*Mp == '#') || (end && !strnicmp(end, Mp, strlen(end)))) {
			Mp = NULL;
			break;
		}

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (!Mp || (*Mp == EOF_CHAR)) {
		diag_printf("Required string [%s] not found\n", pstr);
		Mp = backup;
		Token_found_flag = 0;
		return 0;
	}

	Mp += strlen(pstr);
	diag_printf("Found required string [%s]\n", pstr);
	Token_found_flag = 1;
	return 1;
}

// attempt to find token in buffer.  It might not exist, however, in which case we don't need
// to do anything.  If it is found, then we advance the pointer to just after the token.  To
// further complicate things, we should only search to a certain point, since we don't want
// a token that belongs to another section which might match the token we want.  Thus, we
// also pass in an ending token, which marks the point we should stop looking at.
int optional_string_fred(char *pstr, char *end, char *end2)
{
	char *mp_save = Mp;

	token_found = pstr;
	if (fred_parse_flag)
		return 0;

	ignore_white_space();
	while ((*Mp != EOF_CHAR) && strnicmp(pstr, Mp, strlen(pstr))) {
		if ((*Mp == '#') || (end && !strnicmp(end, Mp, strlen(end))) ||
			(end2 && !strnicmp(end2, Mp, strlen(end2)))) {
			Mp = NULL;
			break;
		}

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (!Mp || (*Mp == EOF_CHAR)) {
		diag_printf("Optional string [%s] not found\n", pstr);
		Mp = mp_save;
		Token_found_flag = 0;
		return 0;
	}

	Mp += strlen(pstr);
	diag_printf("Found optional string [%s]\n", pstr);
	Token_found_flag = 1;
	return 1;
}

//	Return 0 or 1 for str1 match, str2 match.  Return -1 if neither matches.
//	Does not update Mp if token found.  If not found, advances, trying to
//	find the string.  Doesn't advance past the found string.
int required_string_either(char *str1, char *str2)
{
	int	count = 0;

	ignore_white_space();

	while (count < RS_MAX_TRIES) {
		if (strnicmp(str1, Mp, strlen(str1)) == 0) {
			// Mp += strlen(str1);
			diag_printf("Found required string [%s]\n%s", token_found = str1, parse_error_text);
			return 0;
		} else if (strnicmp(str2, Mp, strlen(str2)) == 0) {
			// Mp += strlen(str2);
			diag_printf("Found required string [%s]\n%s", token_found = str2, parse_error_text);
			return 1;
		}

		error_display(1, "Required token = [%s] or [%s], found [%.32s] %s.\n", str1, str2, next_tokens(), parse_error_text);

		advance_to_eoln(NULL);
		ignore_white_space();
		count++;
	}

	if (count == RS_MAX_TRIES) {
		nprintf(("Error", "Error: Unable to find either required token [%s] or [%s]\n", str1, str2));
		Warning(LOCATION, "Error: Unable to find either required token [%s] or [%s]\n", str1, str2);
		longjmp(parse_abort, 2);
	}

	return -1;
	// exit (1);
}

//	Return 0 or 1 for str1 match, str2 match.  Return -1 if neither matches.
//	Does not update Mp if token found.  If not found, advances, trying to
//	find the string.  Doesn't advance past the found string.
int required_string_3(char *str1, char *str2, char *str3)
{
	int	count = 0;

	ignore_white_space();

	while (count < RS_MAX_TRIES) {
		if (strnicmp(str1, Mp, strlen(str1)) == 0) {
			// Mp += strlen(str1);
			diag_printf("Found required string [%s]\n", token_found = str1);
			return 0;
		} else if (strnicmp(str2, Mp, strlen(str2)) == 0) {
			// Mp += strlen(str2);
			diag_printf("Found required string [%s]\n", token_found = str2);
			return 1;
		} else if (strnicmp(str3, Mp, strlen(str3)) == 0) {
			diag_printf("Found required string [%s]\n", token_found = str3);
			return 2;
		}

		error_display(1, "Required token = [%s], [%s] or [%s], found [%.32s].\n", str1, str2, str3, next_tokens());

		advance_to_eoln(NULL);
		ignore_white_space();
		count++;
	}

	return -1;
	// exit (1);
}

int required_string_4(char *str1, char *str2, char *str3, char *str4)
{
	int	count = 0;
	
	ignore_white_space();
	
	while (count < RS_MAX_TRIES) {
		if (strnicmp(str1, Mp, strlen(str1)) == 0) {
			// Mp += strlen(str1);
			diag_printf("Found required string [%s]\n", token_found = str1);
			return 0;
		} else if (strnicmp(str2, Mp, strlen(str2)) == 0) {
			// Mp += strlen(str2);
			diag_printf("Found required string [%s]\n", token_found = str2);
			return 1;
		} else if (strnicmp(str3, Mp, strlen(str3)) == 0) {
			diag_printf("Found required string [%s]\n", token_found = str3);
			return 2;
		} else if (strnicmp(str4, Mp, strlen(str4)) == 0) {
			diag_printf("Found required string [%s]\n", token_found = str4);
			return 3;
		}
		
		error_display(1, "Required token = [%s], [%s], [%s], or [%s], found [%.32s].\n", str1, str2, str3, str4, next_tokens());
		
		advance_to_eoln(NULL);
		ignore_white_space();
		count++;
	}
	
	return -1;
	// exit (1);
}

int required_string_either_fred(char *str1, char *str2)
{
	ignore_white_space();

	while (*Mp != EOF_CHAR) {
		if (!strnicmp(str1, Mp, strlen(str1))) {
			// Mp += strlen(str1);
			diag_printf("Found required string [%s]\n", token_found = str1);
			return fred_parse_flag = 0;
		
		} else if (!strnicmp(str2, Mp, strlen(str2))) {
			// Mp += strlen(str2);
			diag_printf("Found required string [%s]\n", token_found = str2);
			return fred_parse_flag = 1;
		}

		advance_to_eoln(NULL);
		ignore_white_space();
	}

	if (*Mp == EOF_CHAR)
		diag_printf("Unable to find either required token [%s] or [%s]\n", str1, str2);

	return -1;
	// exit (1);
}

//	Copy characters from instr to outstr until eoln is found, or until max
//	characters have been copied (including terminator).
void copy_to_eoln(char *outstr, char *more_terminators, char *instr, int max)
{
	int	count = 0;
	char	ch;
	char	terminators[128];

	Assert((more_terminators == NULL) || (strlen(more_terminators) < 125));

	terminators[0] = EOLN;
	terminators[1] = (char)EOF_CHAR;
	terminators[2] = 0;
	if (more_terminators != NULL)
		strcat_s(terminators, more_terminators);
	else
		terminators[2] = 0;

	while (((ch = *instr++) != 0) && (strchr(terminators, ch) == NULL)  && (count < max)) {
		*outstr++ = ch;
		count++;
	}

	if (count >= max)
		error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", next_tokens(), strlen(next_tokens()), max);

	*outstr = 0;
}

//	Ditto for SCP_string.
void copy_to_eoln(SCP_string &outstr, char *more_terminators, char *instr)
{
	char	ch;
	char	terminators[128];

	Assert((more_terminators == NULL) || (strlen(more_terminators) < 125));

	terminators[0] = EOLN;
	terminators[1] = (char)EOF_CHAR;
	terminators[2] = 0;
	if (more_terminators != NULL)
		strcat_s(terminators, more_terminators);
	else
		terminators[2] = 0;

	outstr = "";
	while (((ch = *instr++) != 0) && (strchr(terminators, ch) == NULL)) {
		outstr.append(1, ch);
	}
}

//	Copy characters from instr to outstr until next white space is found, or until max
//	characters have been copied (including terminator).
void copy_to_next_white(char *outstr, char *instr, int max)
{
	int	count = 0;
	int	in_quotes = 0;
	char	ch;

	while (((ch = *instr++)>0) && (ch != EOLN) && (ch != EOF_CHAR) && (count < max)) {
		if ( ch == '\"' ) {
			in_quotes = !in_quotes;
			continue;
		}

		if ( !in_quotes && is_white_space(ch) )	// not in quotes, white space terminates string
			break;

		if ( !in_quotes && is_parenthesis(ch) ) // not in quotes, parentheses are important for parsing so we don't want to copy them
			break;

		*outstr++ = ch;
		count++;
	}

	if (count >= max)
		error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", next_tokens(), strlen(next_tokens()), max);

	*outstr = 0;
}

//	Ditto for SCP_string.
void copy_to_next_white(SCP_string &outstr, char *instr)
{
	int	in_quotes = 0;
	char	ch;

	outstr = "";
	while (((ch = *instr++)>0) && (ch != EOLN) && (ch != EOF_CHAR)) {
		if ( ch == '\"' ) {
			in_quotes = !in_quotes;
			continue;
		}

		if ( !in_quotes && is_white_space(ch) )	// not in quotes, white space terminates string
			break;

		if ( !in_quotes && is_parenthesis(ch) ) // not in quotes, parentheses are important for parsing so we don't want to copy them
			break;

		outstr.append(1, ch);
	}
}

//Returns a null-terminated character string allocated with vm_malloc() with the data
char* alloc_text_until(char* instr, char* endstr)
{
	Assert(instr && endstr);
	char *foundstr = stristr(instr, endstr);
	if(foundstr == NULL)
	{
		Error(LOCATION, "Missing [%s] in file");
		longjmp(parse_abort, 3);
	}
	else
	{
		char* rstr = NULL;
		rstr = (char*) vm_malloc((foundstr - instr)*sizeof(char));

		if(rstr != NULL) {
			strncpy(rstr, instr, foundstr-instr);
			rstr[foundstr-instr] = '\0';
		} else {
			Error(LOCATION, "Could not allocate enough memory in alloc_text_until");
		}

		return rstr;
	}
}

//	Copy text until a certain string is matched.
//	For example, this is used to copy mission notes, scanning until $END NOTES:
// is found.
void copy_text_until(char *outstr, char *instr, char *endstr, int max_chars)
{
	char *foundstr;
	Assert(outstr && instr && endstr);

	foundstr = stristr(instr, endstr);

	if (foundstr == NULL) {
		nprintf(("Error", "Error.  Looking for [%s], but never found it.\n", endstr));
		longjmp(parse_abort, 3);
	}

	if (foundstr - instr + strlen(endstr) < (uint) max_chars) {
		strncpy(outstr, instr, foundstr - instr);
		outstr[foundstr - instr] = 0;

	} else {
		nprintf(("Error", "Error.  Too much text (%i chars, %i allowed) before %s\n",
			foundstr - instr - strlen(endstr), max_chars, endstr));

		longjmp(parse_abort, 4);
	}

	diag_printf("Here's the partial wad of text:\n%.30s\n", outstr);
}

//	Ditto for SCP_string.
void copy_text_until(SCP_string &outstr, char *instr, char *endstr)
{
	char *foundstr;
	Assert(instr && endstr);

	foundstr = stristr(instr, endstr);

	if (foundstr == NULL) {
		nprintf(("Error", "Error.  Looking for [%s], but never found it.\n", endstr));
		longjmp(parse_abort, 3);
	}

	outstr.assign(instr, foundstr - instr);

	diag_printf("Here's the partial wad of text:\n%.30s\n", outstr.c_str());
}

// stuffs a string into a buffer.  Can get a string between " marks and stops
// when whitespace is encounted -- not to end of line
void stuff_string_white(char *outstr, int len)
{
	if(!len)
		len = NAME_LENGTH-1;

	ignore_white_space();
	copy_to_next_white(outstr, Mp, len);
	advance_to_next_white();
}

// ditto for SCP_string
void stuff_string_white(SCP_string &outstr)
{
	ignore_white_space();
	copy_to_next_white(outstr, Mp);
	advance_to_next_white();
}

// Goober5000
void stuff_string_until(char *outstr, char *endstr, int len)
{
	if(!len)
		len = NAME_LENGTH-1;

	ignore_gray_space();
	copy_text_until(outstr, Mp, endstr, len);
	Mp += strlen(outstr);
	drop_trailing_white_space(outstr);
}

// Goober5000
void stuff_string_until(SCP_string &outstr, char *endstr)
{
	ignore_gray_space();
	copy_text_until(outstr, Mp, endstr);
	Mp += outstr.length();
	drop_trailing_white_space(outstr);
}

//WMC
//Used for allocating large blocks, eg of Python code
//Returns a null-terminated string allocated with vm_malloc(),
//or NULL on failure
//Does depth checks for the start and end strings
//extra_chars indicates extra malloc space that should be allocated.
char* alloc_block(char* startstr, char* endstr, int extra_chars)
{
	Assert(startstr != NULL && endstr != NULL);
	Assert(stricmp(startstr, endstr));

	char* rval = NULL;
	uint elen = strlen(endstr);
	uint slen = strlen(startstr);
	uint flen = 0;

	//Skip the opening thing and any extra stuff
	required_string(startstr);
	ignore_white_space();

	//Allocate it
	char* pos = Mp;

	//Depth checking
	int level = 1;
	while(*pos != EOF_CHAR)
	{
		if(!strnicmp(pos, startstr, slen))
		{
			level++;
		}
		else if(!strnicmp(pos, endstr, elen))
		{
			level--;
		}

		if(level<=0)
		{
			break;
		}

		pos++;
	}

	//Check that we left the file
	if(level > 0)
	{
		Error(LOCATION, "Unclosed pair of \"%s\" and \"%s\" on line %d in file", startstr, endstr, get_line_num());
		longjmp(parse_abort, 3);
	}
	else
	{
		//Set final length for faster calcs
		flen = pos-Mp;

		//Allocate the memory
		//WMC - Don't forget the null character that's added later on.
		rval = (char*) vm_malloc((flen + extra_chars + 1)*sizeof(char));

		//Copy the text (if memory was allocated)
		if(rval != NULL) {
			strncpy(rval, Mp, flen);
			rval[flen] = '\0';
		} else {
			return NULL;
		}
	}

	//Skip the copied stuff
	Mp += flen;
	required_string(endstr);
	return rval;
}

// Karajorma - Stuffs the provided char array with either the contents of a quoted string or the name of a string 
// variable. Returns PARSING_FOUND_STRING if a string was found or PARSING_FOUND_VARIABLE if a variable was present. 
int get_string_or_variable (char *str)
{
	int result = -1; 

	ignore_white_space();
	
	// Variable
	if (*Mp == '@') 
	{
		Mp++;
		stuff_string_white(str); 
		int sexp_variable_index = get_index_sexp_variable_name(str); 
		
		// We only want String variables
		Assert (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING);

		result = PARSING_FOUND_VARIABLE; 
	}
	// Quoted string
	else if (*Mp == '"')
	{
		get_string(str);
		result =  PARSING_FOUND_STRING;
	}
	else
	{
		get_string(str);
		Error(LOCATION, "Invalid entry \"%s\"  found in get_string_or_variable. Must be a quoted string or a string variable name.", str); 
	}

	return result;
}

// ditto for SCP_string
int get_string_or_variable (SCP_string &str)
{
	int result = -1; 

	ignore_white_space();
	
	// Variable
	if (*Mp == '@') 
	{
		Mp++;
		stuff_string_white(str); 
		int sexp_variable_index = get_index_sexp_variable_name(str); 
		
		// We only want String variables
		Assert (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING);

		result = PARSING_FOUND_VARIABLE; 
	}
	// Quoted string
	else if (*Mp == '"')
	{
		get_string(str);
		result =  PARSING_FOUND_STRING;
	}
	else
	{
		get_string(str);
		Error(LOCATION, "Invalid entry \"%s\"  found in get_string_or_variable. Must be a quoted string or a string variable name.", str.c_str()); 
	}

	return result;
}

/**
 * Stuff a string (" chars ") into *str, return length.
 */
int get_string(char *str)
{
	int	len;

	len = strcspn(Mp + 1, "\"");
	strncpy(str, Mp + 1, len);
	str[len] = 0;

	Mp += len + 2;
	return len;
}

/**
 * Stuff a string (" chars ") into str.
 */
void get_string(SCP_string &str)
{
	int len;

	len = strcspn(Mp + 1, "\"");
	str.assign(Mp + 1, len);

	Mp += len + 2;
}

//	Stuff a string into a string buffer.
//	Supports various FreeSpace primitive types.  If 'len' is supplied, it will override
// the default string length if using the F_NAME case.
void stuff_string(char *outstr, int type, int len, char *terminators)
{
	char read_str[PARSE_BUF_SIZE] = "";
	int read_len = PARSE_BUF_SIZE;
	int final_len = len - 1;
	int tag_id;

	// make sure we have enough room
	Assert( final_len > 0 );

	// make sure it's zero'd out
	memset( outstr, 0, len );

	switch (type) {
		case F_RAW:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_LNAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_NAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_DATE:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_NOTES:
			ignore_white_space();
			copy_text_until(read_str, Mp, "$End Notes:", read_len);
			Mp += strlen(read_str);
			required_string("$End Notes:");
			break;

		case F_FILESPEC:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		// F_MULTITEXTOLD keeping for backwards compatability with old missions
		// can be deleted once all missions are using new briefing format

		case F_MULTITEXTOLD:		
			ignore_white_space();
			copy_text_until(read_str, Mp, "$End Briefing Text:", read_len);
			Mp += strlen(read_str);
			required_string("$End Briefing Text:");
			break;

		case F_MULTITEXT:		
			ignore_white_space();
			copy_text_until(read_str, Mp, "$end_multi_text", read_len);
			Mp += strlen(read_str);
			drop_trailing_white_space(read_str);
			required_string("$end_multi_text");
			break;

		case F_PATHNAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_MESSAGE:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp, read_len);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;		

		default:
			Error(LOCATION, "Unhandled string type %d in stuff_string!", type);
	}

	// now we want to do any final localization
	if(type != F_RAW && type != F_LNAME)
	{
		lcl_ext_localize(read_str, outstr, final_len, &tag_id);

		// if the hash localized text hash table is active and we have a valid external string - hash it
		if(fhash_active() && (tag_id > -2)){
			fhash_add_str(outstr, tag_id);
		}
	}
	else
	{
		if ( strlen(read_str) > (uint)final_len )
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", read_str, strlen(read_str), final_len);

		strncpy(outstr, read_str, final_len);
	}

	diag_printf("Stuffed string = [%.30s]\n", outstr);
}

//	Stuff a string into a string buffer.
//	Supports various FreeSpace primitive types.
void stuff_string(SCP_string &outstr, int type, char *terminators)
{
	SCP_string read_str;
	int tag_id;

	// make sure it's zero'd out
	outstr = "";

	switch (type) {
		case F_RAW:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_LNAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_NAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_DATE:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_NOTES:
			ignore_white_space();
			copy_text_until(read_str, Mp, "$End Notes:");
			Mp += read_str.length();
			required_string("$End Notes:");
			break;

		case F_FILESPEC:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		// F_MULTITEXTOLD keeping for backwards compatability with old missions
		// can be deleted once all missions are using new briefing format

		case F_MULTITEXTOLD:		
			ignore_white_space();
			copy_text_until(read_str, Mp, "$End Briefing Text:");
			Mp += read_str.length();
			required_string("$End Briefing Text:");
			break;

		case F_MULTITEXT:		
			ignore_white_space();
			copy_text_until(read_str, Mp, "$end_multi_text");
			Mp += read_str.length();
			drop_trailing_white_space(read_str);
			required_string("$end_multi_text");
			break;

		case F_PATHNAME:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;

		case F_MESSAGE:
			ignore_gray_space();
			copy_to_eoln(read_str, terminators, Mp);
			drop_trailing_white_space(read_str);
			advance_to_eoln(terminators);
			break;		

		default:
			Error(LOCATION, "Unhandled string type %d in stuff_string!", type);
	}

	// now we want to do any final localization
	if(type != F_RAW && type != F_LNAME)
	{
		lcl_ext_localize(read_str, outstr, &tag_id);

		// if the hash localized text hash table is active and we have a valid external string - hash it
		if(fhash_active() && (tag_id > -2)){
			fhash_add_str(outstr.c_str(), tag_id);
		}
	}
	else
	{
		outstr = read_str;
	}

	diag_printf("Stuffed string = [%.30s]\n", outstr.c_str());
}

// stuff a string, but only until the end of a line. don't ignore leading whitespace. close analog of fgets()/cfgets()
void stuff_string_line(char *outstr, int len)
{
	char read_str[PARSE_BUF_SIZE] = "";
	int read_len = PARSE_BUF_SIZE;
	int final_len = len - 1;
	int tag_id;

	Assert( final_len > 0 );

	// read in a line
	copy_to_eoln(read_str, "\n", Mp, read_len);
	drop_trailing_white_space(read_str);
	advance_to_eoln("");
	Mp++;

	// now we want to do any final localization
	lcl_ext_localize(read_str, outstr, final_len, &tag_id);

	// if the hash localized text hash table is active and we have a valid external string - hash it
	if(fhash_active() && (tag_id > -2)){
		fhash_add_str(outstr, tag_id);
	}

	diag_printf("Stuffed string = [%.30s]\n", outstr);
}

// ditto for SCP_string
void stuff_string_line(SCP_string &outstr)
{
	SCP_string read_str;
	int tag_id;

	// read in a line
	copy_to_eoln(read_str, "\n", Mp);
	drop_trailing_white_space(read_str);
	advance_to_eoln("");
	Mp++;

	// now we want to do any final localization
	lcl_ext_localize(read_str, outstr, &tag_id);

	// if the hash localized text hash table is active and we have a valid external string - hash it
	if(fhash_active() && (tag_id > -2)){
		fhash_add_str(outstr.c_str(), tag_id);
	}

	diag_printf("Stuffed string = [%.30s]\n", outstr.c_str());
}

// Exactly the same as stuff string only Malloc's the buffer. 
//	Supports various FreeSpace primitive types.  If 'len' is supplied, it will override
// the default string length if using the F_NAME case.
char *stuff_and_malloc_string(int type, char *terminators)
{
	SCP_string tmp_result;

	stuff_string(tmp_result, type, terminators);
	drop_white_space(tmp_result);

	if (tmp_result.empty())
		return NULL;

	return vm_strdup(tmp_result.c_str());
}

void stuff_malloc_string(char **dest, int type, char *terminators)
{
	Assert(dest != NULL); //wtf?
	
	char *new_val = stuff_and_malloc_string(type, terminators);
	
	if(new_val != NULL)
	{
		if((*dest) != NULL) {
			vm_free(*dest);
		}
		
		(*dest) = new_val;
	}
}

// After reading a multitext string, you can call this function to convert any newlines into
// spaces, so it's a one paragraph string (I.e. as in MS-Word).
//
void compact_multitext_string(char *str)
{
	unsigned int i;
	unsigned int len = strlen(str);
	int num_cr = 0;

	for (i=0; i<len; i++)
	{
		// skip CR
		// convert LF to space
		// copy characters backwards if any CRs previously encountered
		if (str[i] == '\r')
			num_cr++;
		else if (str[i] == '\n')
			str[i-num_cr] = ' ';
		else if (num_cr > 0)
			str[i-num_cr] = str[i];
	}

	if (num_cr > 0)
		str[len-num_cr] = 0;
}

// ditto for SCP_string
void compact_multitext_string(SCP_string &str)
{
	unsigned int i;
	unsigned int len = str.length();
	int num_cr = 0;

	for (i=0; i<len; i++)
	{
		// skip CR
		// convert LF to space
		// copy characters backwards if any CRs previously encountered
		if (str[i] == '\r')
			num_cr++;
		else if (str[i] == '\n')
			str[i-num_cr] = ' ';
		else if (num_cr > 0)
			str[i-num_cr] = str[i];
	}

	if (num_cr > 0)
		str.resize(len-num_cr);
}

int maybe_convert_foreign_character(int ch)
{
	// time to do some special foreign character conversion			
	switch (ch) {
		case -4:
			ch = 129;
			break;

		case -28:
			ch = 132;
			break;

		case -10:
			ch = 148;
			break;

		case -23:
			ch = 130;
			break;

		case -30:
			ch = 131;
			break;

		case -25:
			ch = 135;
			break;

		case -21:
			ch = 137;
			break;

		case -24:
			ch = 138;
			break;

		case -17:
			ch = 139;
			break;

		case -18:
			ch = 140;
			break;

		case -60:
			ch = 142;
			break;

		case -55:
			ch = 144;
			break;

		case -12:
			ch = 147;
			break;

		case -14:
			ch = 149;
			break;

		case -5:
			ch = 150;
			break;

		case -7:
			ch = 151;
			break;

		case -42:
			ch = 153;
			break;

		case -36:
			ch = 154;
			break;

		case -31:
			ch = 160;
			break;

		case -19:
			ch = 161;
			break;

		case -13:
			ch = 162;
			break;

		case -6:
			ch = 163;
			break;

		case -32:
			ch = 133;
			break;

		case -22:
			ch = 136;
			break;

		case -20:
			ch = 141;
			break;
	}
	
	return ch;
}

// Goober5000
void maybe_convert_foreign_characters(char *line)
{
	char *ch;
	if (Fred_running)
		return;

	for (ch = line; *ch != '\0'; ch++)
			*ch = (char) maybe_convert_foreign_character(*ch);
}

// Goober5000
void maybe_convert_foreign_characters(SCP_string &line)
{
	if (Fred_running)
		return;

	for (SCP_string::iterator ii = line.begin(); ii != line.end(); ++ii)
		*ii = (char) maybe_convert_foreign_character(*ii);
}

// Goober5000
int get_number_before_separator(char *text, char separator)
{
	char buf[10];
	char *ch;

	memset(buf, 0, 10);
	strncpy(buf, text, 9);

	ch = strchr(buf, separator);
	if (ch == NULL)
		return 0;
	*ch = '\0';

	return atoi(buf);	
}

// Goober5000
int get_number_before_separator(SCP_string &text, char separator)
{
	char buf[10];
	char *ch;

	memset(buf, 0, 10);
	text.copy(buf, 9);

	ch = strchr(buf, separator);
	if (ch == NULL)
		return 0;
	*ch = '\0';

	return atoi(buf);	
}

// Strip comments from a line of input.
// Goober5000 - rewritten to make a lot more sense and to be extensible
int strip_comments(char *line, int in_multiline_comment)
{
	char *ch;

	// if we're in a comment, see if we can close it
	if (in_multiline_comment)
	{
		ch = strstr(line, "*/");
		if (ch != NULL)
		{
			char *writep = line;
			char *readp = ch + 2;

			// copy all characters past the close of the comment
			while (*readp != '\0')
			{
				*writep = *readp;

				writep++;
				readp++;
			}

			*writep = '\0';

			// recurse with the other characters
			return strip_comments(line, 0);
		}

		// can't close it, so drop the whole line
		ch = line;
		goto done_with_line;
	}


	// start of a multi-line comment?
	ch = strstr(line, "/*");
	if (ch != NULL)
	{
		// treat it as the beginning of a new line and recurse
		return strip_comments(ch, 1);
	}


	/* Goober5000 - this interferes with hyperlinks, heh
	// search for //
	ch = strstr(line, "//");
	if (ch != NULL)
		goto done_with_line;
	*/


	// special version-specific comment
	// formatted like e.g. ;;FSO 3.7.0;;
	ch = stristr(line, ";;FSO ");
	if (ch != NULL)
	{
		int major, minor, build;
		char *numch, *sep, *linech;

		numch = ch + 6;
		sep = strchr(numch, '.');
		if (sep == NULL)
			goto done_with_line;

		major = get_number_before_separator(numch, '.');

		numch = sep + 1;
		sep = strchr(numch, '.');
		if (sep == NULL)
			goto done_with_line;

		minor = get_number_before_separator(numch, '.');

		numch = sep + 1;
		sep = strchr(numch, ';');
		if (sep == NULL)
			goto done_with_line;

		build = get_number_before_separator(numch, ';');

		if (*(sep + 1) != ';')
			goto done_with_line;

		linech = sep + 2;


		// check whether major, minor, and build line up with this version
		if (major > FS_VERSION_MAJOR)
			goto done_with_line;
		if (minor > FS_VERSION_MINOR)
			goto done_with_line;
		if (build > FS_VERSION_BUILD)
			goto done_with_line;

	
		// this version is compatible, so copy the line past the tag
		{
			char *writep = line;
			char *readp = linech;

			// copy all characters past the close of the comment
			while (*readp != '\0')
			{
				*writep = *readp;

				writep++;
				readp++;
			}

			*writep = '\0';

			// recurse with the other characters
			return strip_comments(line, 0);
		}
	}


	// search for ;
	ch = strchr(line, ';');
	if (ch != NULL)
		goto done_with_line;


	// no comments found... try to find the newline
	ch = strchr(line, '\n');
	if (ch != NULL)
		goto done_with_line;


	// just skip to the end of the line
	ch = line + strlen(line);


done_with_line:
	ch[0] = EOLN;
	ch[1] = 0;

	return in_multiline_comment;	
}

/*#if 0
void strip_all_comments( char *readp, char *writep )
{
	int	ch;
	//char	*writep = readp;

	while ( *readp != EOF_CHAR ) {
		ch = *readp;
		if ( ch == COMMENT_CHAR ) {
			while ( *readp != EOLN )
				readp++;

			*writep = EOLN;
			writep++;
			// get to next character after EOLN
			readp++;
		} else if ( (ch == '/') && (readp[1] == '*')) {			// Start of multi-line comment
			int done;
			
			done = 0;
			while ( !done ) {
				while ( *readp != '*' )
					readp++;
				if ( readp[1] == '/' ) {
					readp += 2;
					done = 1;
				} else {
					readp++;
				}
			}
		} else {
			*writep = (char)ch;
			*writep++;
			readp++;
		}
	}

	*writep = (char)EOF_CHAR;
}
#endif*/

int parse_get_line(char *lineout, int max_line_len, char *start, int max_size, char *cur)
{
	char * t = lineout;
	int i, num_chars_read=0;
	char c;


	for ( i = 0; i < max_line_len-1; i++ ) {
		do {
			if ( (cur - start) >= max_size ) {
				*lineout = 0;
				if ( lineout > t ) {		
					return num_chars_read;
				} else {
					return 0;
				}
			}
			c = *cur++;
			num_chars_read++;
		} while ( c == 13 );

		*lineout++ = c;
		if ( c=='\n' ) break;
	}

	*lineout++ = 0;
	return  num_chars_read;
}

//	Read mission text, stripping comments.
//	When a comment is found, it is removed.  If an entire line
//	consisted of a comment, a blank line is left in the input file.
// Goober5000 - added ability to read somewhere other than Mission_text
void read_file_text(char *filename, int mode, char *processed_text, char *raw_text)
{
	// copy the filename
	if (!filename)
		longjmp(parse_abort, 10);
	strcpy_s(Current_filename_sub, filename);

	// if we are paused then processed_text and raw_text must not be NULL!!
	if ( Parsing_paused && ((processed_text == NULL) || (raw_text == NULL)) ) {
		Error(LOCATION, "ERROR: Neither processed_text nor raw_text may be NULL when parsing is paused!!\n");
	}

	// read the raw text
	read_raw_file_text(filename, mode, raw_text);

	if (processed_text == NULL)
		processed_text = Mission_text;

	if (raw_text == NULL)
		raw_text = Mission_text_raw;

	// process it (strip comments)
	process_raw_file_text(processed_text, raw_text);
}

// Goober5000
void read_file_text_from_array(char *array, char *processed_text, char *raw_text)
{
	// we have no filename, so copy a substitute
	strcpy_s(Current_filename_sub, "internal default file");

	// if we are paused then processed_text and raw_text must not be NULL!!
	if ( Parsing_paused && ((processed_text == NULL) || (raw_text == NULL)) ) {
		Error(LOCATION, "ERROR: Neither \"processed_text\" nor \"raw_text\" may be NULL when parsing is paused!!\n");
	}

	// make sure to do this before anything else
	allocate_mission_text( strlen(array) + 1 );

	// if we have no raw buffer, set it as the default raw text area
	if (raw_text == NULL)
		raw_text = Mission_text_raw;

	// copy text in the array (but only if the raw text and the array are not the same)
	if (raw_text != array)
		strcpy(raw_text, array);

	if (processed_text == NULL)
		processed_text = Mission_text;

	// process the text
	process_raw_file_text(processed_text, raw_text);
}

// Goober5000
int is_unicode(char *text)
{
	if (!strncmp(text, "\xEF\xBB\xBF", 3))		// UTF-8
		return 1;

	if (!strncmp(text, "\xFE\xFF", 2))			// UTF-16 big-endian
		return 1;

	if (!strncmp(text, "\xFF\xFE", 2))			// UTF-16 little-endian
		return 1;

	if (!strncmp(text, "\x00\x00\xFE\xFF", 4))	// UTF-32 big-endian
		return 1;

	if (!strncmp(text, "\xFF\xFE\x00\x00", 4))	// UTF-32 little-endian
		return 1;

	return 0;
}

void stop_parse()
{
	Assert( !Parsing_paused );

	if (Mission_text != NULL) {
		vm_free(Mission_text);
		Mission_text = NULL;
	}

	if (Mission_text_raw != NULL) {
		vm_free(Mission_text_raw);
		Mission_text_raw = NULL;
	}

	Mission_text_size = 0;
}

void allocate_mission_text(int size)
{
	Assert( size > 0 );

	if (size <= Mission_text_size)
		return;


	static ubyte parse_atexit = 0;

	if (!parse_atexit) {
		atexit(stop_parse);
		parse_atexit = 1;
	}

	if (Mission_text != NULL) {
		vm_free(Mission_text);
		Mission_text = NULL;
	}

	if (Mission_text_raw != NULL) {
		vm_free(Mission_text_raw);
		Mission_text_raw = NULL;
	}

	Mission_text = (char *) vm_malloc_q(sizeof(char) * size);
	Mission_text_raw = (char *) vm_malloc_q(sizeof(char) * size);

	if ( (Mission_text == NULL) || (Mission_text_raw == NULL) ) {
		Error(LOCATION, "Unable to allocate enough memory for Mission_text!  Aborting...\n");
	}

	memset( Mission_text, 0, sizeof(char) * size );
	memset( Mission_text_raw, 0, sizeof(char) * size);

	Mission_text_size = size;
}

// Goober5000
void read_raw_file_text(char *filename, int mode, char *raw_text)
{
	CFILE	*mf;
	int	file_is_encrypted;
	int file_is_unicode;

	Assert(filename);

	mf = cfopen(filename, "rb", CFILE_NORMAL, mode);
	if (mf == NULL)
	{
		nprintf(("Error", "Wokka!  Error opening file (%s)!\n", filename));
		longjmp(parse_abort, 5);
	}

	// read the entire file in
	int file_len = cfilelength(mf);

	if(!file_len) {
		nprintf(("Error", "Oh noes!!  File is empty! (%s)!\n", filename));
		longjmp(parse_abort, 5);
	}

	// allocate, or reallocate, memory for Mission_text and Mission_text_raw based on size we need now
	allocate_mission_text( file_len + 1 );

	// NOTE: this always has to be done *after* the allocate_mission_text() call!!
	if (raw_text == NULL)
		raw_text = Mission_text_raw;

	// read first 10 bytes to determine if file is encrypted
	cfread(raw_text, MIN(file_len, 10), 1, mf);
	file_is_encrypted = is_encrypted(raw_text);
	cfseek(mf, 0, CF_SEEK_SET);

	// Goober5000 - also determine if file is Unicode
	file_is_unicode = is_unicode(raw_text);
	if ( file_is_unicode )
	{
		nprintf(("Error", "Wokka!  File (%s) is in Unicode format!\n", filename));
		longjmp(parse_abort, 5);
	}

	if ( file_is_encrypted )
	{
		int	unscrambled_len;
		char	*scrambled_text;
		scrambled_text = (char*)vm_malloc(file_len+1);
		Assert(scrambled_text);
		cfread(scrambled_text, file_len, 1, mf);
		// unscramble text
		unencrypt(scrambled_text, file_len, raw_text, &unscrambled_len);
		file_len = unscrambled_len;
		vm_free(scrambled_text);
	}
	else
	{
		cfread(raw_text, file_len, 1, mf);
	}

	//WMC - Slap a NULL character on here for the odd error where we forgot a #End
	raw_text[file_len] = '\0';

	cfclose(mf);
}

// Goober5000
void process_raw_file_text(char *processed_text, char *raw_text)
{
	char	*mp;
	char	*mp_raw;
	char outbuf[PARSE_BUF_SIZE], *str;
	int in_multiline_comment = 0;
	int raw_text_len = strlen(raw_text);

	if (processed_text == NULL)
		processed_text = Mission_text;

	if (raw_text == NULL)
		raw_text = Mission_text_raw;

	Assert( processed_text != NULL );
	Assert( raw_text != NULL );

	mp = processed_text;
	mp_raw = raw_text;

	// strip comments from raw text, reading into file_text
	int num_chars_read = 0;
	while ( (num_chars_read = parse_get_line(outbuf, PARSE_BUF_SIZE, raw_text, raw_text_len, mp_raw)) != 0 ) {
		mp_raw += num_chars_read;

		in_multiline_comment = strip_comments(outbuf, in_multiline_comment);

		maybe_convert_foreign_characters(outbuf);

		str = outbuf;
		while (*str) {
			if (*str == -33) {
				*mp++ = 's';
				*mp++ = 's';
				str++;

			} else
				*mp++ = *str++;
		}

//		strcpy_s(mp, outbuf);
//		mp += strlen(outbuf);
	}

	*mp = *mp_raw = (char)EOF_CHAR;
/*
	while (cfgets(outbuf, PARSE_BUF_SIZE, mf) != NULL) {
		if (strlen(outbuf) >= PARSE_BUF_SIZE-1)
			error_display(0, "Input string too long.  Max is %i characters.\n%.256s\n", PARSE_BUF_SIZE, outbuf);

		//	If you hit this assert, it is probably telling you the obvious.  The file
		//	you are trying to read is truly too large.  Look at *filename to see the file name.
		Assert(mp_raw - file_text_raw + strlen(outbuf) < MISSION_TEXT_SIZE);
		strcpy_s(mp_raw, outbuf);
		mp_raw += strlen(outbuf);

		in_comment = strip_comments(outbuf, in_comment);
		strcpy_s(mp, outbuf);
		mp += strlen(outbuf);
	}
	
	*mp = *mp_raw = (char)EOF_CHAR;
*/

}

void debug_show_mission_text()
{
	char	*mp = Mission_text;
	char	ch;

	while ((ch = *mp++) != EOF_CHAR)
		printf("%c", ch);
}

float atof2()
{
	char	ch;

	my_errno = 0;
	ignore_white_space();

	ch = *Mp;

	if ((ch != '.') && (ch != '-') && (ch != '+') && ((ch < '0') || (ch > '9'))) {
		error_display(1, "Expecting float, found [%.32s].\n", next_tokens());
		my_errno = 1;
		return 0.0f;
	} else
		return (float)atof(Mp);

}

int atoi2()
{
	char	ch;

	my_errno = 0;

	ignore_white_space();

	ch = *Mp;

	if ((ch != '-') && (ch != '+') && ((ch < '0') || (ch > '9'))) {
		error_display(1, "Expecting int, found [%.32s].\n", next_tokens());
		my_errno = 1;
		return 0;
	} else
		return atoi(Mp);

}

//	Stuff a floating point value pointed at by Mp.
//	Advances past float characters.
void stuff_float(float *f)
{
	*f = atof2();

	if (my_errno)
		skip_token();
	else
		Mp += strspn(Mp, "+-0123456789.");

	if (*Mp ==',')
		Mp++;

	diag_printf("Stuffed float: %f\n", *f);
}

int stuff_float_optional(float *f)
{
	int skip_len;
	bool comma = false;
	
	ignore_white_space();
	skip_len = strspn(Mp, "+-0123456789.");
	if(*(Mp+skip_len) == ',') {
		comma = true;
	}
	
	if(skip_len == 0)
	{
		if(comma) {
			Mp++;
			return 1;
		} else {
			return 0;
		}
	}

	stuff_float(f);
	return 2;
}

//	Stuff an integer value pointed at by Mp.
//	Advances past integer characters.
void stuff_int(int *i)
{
	*i = atoi2();

	if (my_errno)
		skip_token();
	else
		Mp += strspn(Mp, "+-0123456789");

	if (*Mp ==',')
		Mp++;

	diag_printf("Stuffed int: %i\n", *i);
}

int stuff_int_or_variable (int &i, bool positive_value = false);
int stuff_int_or_variable (int *ilp, int count, bool positive_value = false);


// Stuffs an int value or the value of a number variable. Returns the index of the variable or NOT_SET_BY_SEXP_VARIABLE.
int stuff_int_or_variable (int &i, bool positive_value)
{
	int index = NOT_SET_BY_SEXP_VARIABLE;

	if (*Mp == '@') 
	{
		Mp++;
		int value = -1; 
		char str[128];
		stuff_string(str, F_NAME, sizeof(str));

		index = get_index_sexp_variable_name(str); 
			
		if (index > -1 && index < MAX_SEXP_VARIABLES) 
		{
			if (Sexp_variables[index].type & SEXP_VARIABLE_NUMBER)
			{
				value = atoi(Sexp_variables[index].text);
			}
			else 
			{
				Error(LOCATION, "Invalid variable type \"%s\" found in mission. Variable must be a number variable!", str);
			}
		}
		else
		{
			
			Error(LOCATION, "Invalid variable name \"%s\" found.", str);
		}

		// zero negative values if requested
		if (positive_value && value < 0)
		{
			value = 0;
		}

		
		// Record the value of the index for FreeSpace 
		i = value;
	}
	else 
	{
		stuff_int(&i);
	}
	return index;
}

// Stuff an integer value pointed at by Mp.If a variable is found instead stuff the value of that variable and record the 
// index of the variable in the following slot.
int stuff_int_or_variable (int *ilp, int count, bool positive_value)
{
	if (*Mp == '@') 
	{
		Mp++;
		int value = -1; 
		char str[128];
		stuff_string(str, F_NAME, sizeof(str));

		int index = get_index_sexp_variable_name(str); 
			
		if (index > -1 && index < MAX_SEXP_VARIABLES) 
		{
			if (Sexp_variables[index].type & SEXP_VARIABLE_NUMBER)
			{
				value = atoi(Sexp_variables[index].text);
			}
			else 
			{ 
				Error(LOCATION, "Invalid variable type \"%s\" found in mission. Variable must be a number variable!", str);
			}
		}
		else
		{
			
			Error(LOCATION, "Invalid variable name \"%s\" found.", str);
		}

		// zero negative values if requested
		if (positive_value && value < 0)
		{
			value = 0;
		}

		
		// Record the value of the index for FreeSpace 
		ilp[count++] = value;
		// Record the index itself because we may need it later.
		ilp[count++] = index;
	}
	else 
	{
		stuff_int(&ilp[count++]);
		// Since we have a numerical value we don't have a SEXP variable index to add for next slot. 
		ilp[count++] = NOT_SET_BY_SEXP_VARIABLE;
	}
	return count;
}


//Stuffs boolean value.
//Passes things off to stuff_boolean(bool)
void stuff_boolean(int *i, bool a_to_eol)
{
	bool tempb;
	stuff_boolean(&tempb, a_to_eol);
	if(tempb)
		*i = 1;
	else
		*i = 0;
}

void stuff_boolean_flag(int *i, int flag, bool a_to_eol)
{
	bool temp;
	stuff_boolean(&temp, a_to_eol);
	if(temp)
		*i |= flag;
	else
		*i &= ~(flag);
}

// Stuffs a boolean value pointed at by Mp.  
// YES/NO (supporting 1/0 now as well)
// Now supports localization :) -WMC

void stuff_boolean(bool *b, bool a_to_eol)
{
	char token[32];
	stuff_string_white(token, sizeof(token)/sizeof(char));
	if(a_to_eol)
		advance_to_eoln(NULL);

	if( isdigit(token[0]))
	{
		if(token[0] != '0')
			*b = true;
		else
			*b = false;
	}
	else
	{
		if(!stricmp(token, "yes")
			|| !stricmp(token, "true")
			|| !stricmp(token, "ja")		//German
			|| !stricmp(token, "Oui")		//French
			|| !stricmp(token, "si")		//Spanish
			|| !stricmp(token, "ita vero")	//Latin
			|| !stricmp(token, "HIja'") || !stricmp(token, "HISlaH"))	//Klingon
		{
			*b = true;
		}
		else if(!stricmp(token, "no")
			|| !stricmp(token, "false")
			|| !stricmp(token, "nein")		//German
			|| !stricmp(token, "Non")		//French
			//I don't know spanish for "no"
			//But according to altavista, spanish for "No" is "no"
			//Go figure.
			|| !stricmp(token, "minime")	//Latin
			|| !stricmp(token, "ghobe'"))	//Klingon
		{
			*b = false;
		}
		else
		{
			*b = false;
			Warning(LOCATION, "Boolean '%s' type unknown; assuming 'no/false'",token);
		}
	}

	diag_printf("Stuffed bool: %s\n", (b) ? NOX("true") : NOX("false"));
}

int stuff_bool_list(bool *blp, int max_bools)
{
	int count = 0;
	bool trash_buf = false;

	ignore_white_space();
	
	if (*Mp != '(') {
		error_display(1, "Reading boolean list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;

	ignore_white_space();

	while(*Mp != ')')
	{
		if(count < max_bools)
		{
			stuff_boolean(&blp[count++], false);
			ignore_white_space();
			
			//Since Bobb has set a precedent, allow commas for bool lists -WMC
			if(*Mp == ',')
			{
				Mp++;
				ignore_white_space();
			}
		}
		else
		{
			trash_buf = true;
			break;
		}
	}

	if(trash_buf)
	{
		error_display(0, "Boolean list has more than allowed arguments; max is %d. Arguments over max will be ignored.", max_bools);
		while(*Mp != ')')
		{
			stuff_boolean(&trash_buf, false);
			ignore_white_space();
		}
	}

	Mp++;

	return count;
}


//	Stuff an integer value pointed at by Mp.
//	Advances past integer characters.
void stuff_ubyte(ubyte *i)
{
	int	temp;

	temp = atoi2();

	*i = (ubyte)temp;

	if (my_errno)
		skip_token();
	else
		Mp += strspn(Mp, "+-0123456789");

	if (*Mp == ',')
		Mp++;

	diag_printf("Stuffed byte: %i\n", *i);
}

int parse_string_flag_list(int *dest, flag_def_list defs[], int defs_size)
{
	Assert(dest!=NULL);	//wtf?

	char (*slp)[NAME_LENGTH] = (char(*)[32])new char[defs_size*NAME_LENGTH];
	int num_strings = stuff_string_list(slp, defs_size);
	int i, j;

	for(i = 0; i < num_strings; i++)
	{
		for(j = 0; j < defs_size; j++)
		{
			if(!stricmp(slp[i], defs[j].name)) {
				(*dest) |= defs[j].def;
			}
		}
	}

	delete[] slp;	//>_>
	//nobody saw that right

	return num_strings;
}

int stuff_string_list(SCP_vector<SCP_string>& slp)
{
	//_asm int 3;
	slp.clear();

	ignore_white_space();

	if ( *Mp != '(' ) {
		error_display(1, "Reading string list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 100);
	}

	Mp++;

	ignore_white_space();

	char buf[NAME_LENGTH];

	while (*Mp != ')') {
		if(*Mp != '\"') {
			error_display(0, "Missing quotation marks in string list.");
		}
		//Assert ( *Mp == '\"' );					// should always be enclosed in quotes

		get_string( buf );
		slp.push_back(SCP_string(buf));
		ignore_white_space();
	}

	Mp++;

	return slp.size();
}

// Stuffs a list of strings
int stuff_string_list(char slp[][NAME_LENGTH], int max_strings)
{
	int count = 0;
	ignore_white_space();

	if ( *Mp != '(' ) {
		error_display(1, "Reading string list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 100);
	}

	Mp++;

	ignore_white_space();

	while (*Mp != ')') {
		Assert ( count < max_strings );
		if(*Mp != '\"') {
			error_display(0, "Missing quotation marks in string list.");
		}
		//Assert ( *Mp == '\"' );					// should always be enclosed in quotes

		if (count < max_strings) {
			get_string( slp[count++] );
		} else {
			char trash[NAME_LENGTH];
			get_string( trash );
		}
		ignore_white_space();
	}

	Mp++;

	return count;
}

//	Stuffs an integer list.
//	This is of the form ( i* )
//	  where i is an integer.
// For example, (1) () (1 2 3) ( 1 ) are legal integer lists.
int stuff_int_list(int *ilp, int max_ints, int lookup_type)
{
	int	count = 0, ok_flag = 1, dummy;
	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading integer list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();

	while (*Mp != ')') {
		Assert(count < max_ints);
		if (*Mp == '"') {
			int num = 0;
			char str[128];

			get_string(str);
			switch (lookup_type) {
				case SHIP_TYPE:
					num = ship_name_lookup(str);	// returns index of Ship[] entry with name
					break;

				case SHIP_INFO_TYPE:
					ok_flag = 1;
					num = ship_info_lookup(str);	// returns index of Ship_info[] entry with name
					if (num < 0)
						ok_flag = 0;
					break;

				case WEAPON_POOL_TYPE:
					ok_flag = 1;
					num = weapon_info_lookup(str);
					if (num < 0)
						ok_flag = 0;
					break;

				case WEAPON_LIST_TYPE:
					num = weapon_info_lookup(str);
					if (num < 0)
						num = -2;
					break;

				case RAW_INTEGER_TYPE:
					num = atoi(str);
					break;

				default:
					Error(LOCATION,"Unknown lookup_type in stuff_int_list");
					break;
			}

			if (ok_flag) {
				if (num == -1) {
					Error(LOCATION, "Unable to find string \"%s\" in stuff_int_list\n\nMany possible sources for this error.  Get a programmer!\n", str);
				} else if (num == -2) {
					if (str[0] != '\0') {
						if(parse_error_text[0] != '\0'){
							Warning(LOCATION, "Unable to find WEAPON_LIST_TYPE string \"%s\" %s.\n", str, parse_error_text);
						}else{
							Warning(LOCATION, "Unable to find WEAPON_LIST_TYPE string \"%s\" in stuff_int_list\n\nMany possible sources for this error.  Get a programmer!\n", str);
						}
					}
				}

				if (num < 0)  // other negatives used to bypass the above error trap, but should be -1
					num = -1;

				if (count < max_ints) {
					ilp[count++] = num;
				}
			}

		} else {
			if (ok_flag && (count < max_ints))
				stuff_int(&ilp[count++]);
			else
				stuff_int(&dummy);
		}
		
		ignore_white_space();
	}

	Mp++;

	return count;
}

// helper for the next function. Removes a broken entry from ship or weapon lists and advances to the next one
void clean_loadout_list_entry()
{
	int dummy; 

	// clean out the broken entry
	ignore_white_space();
	stuff_int_or_variable(dummy);
	ignore_white_space();
}

// Karajorma - Stuffs an int list by parsing a list of ship or weapon choices. 
// Unlike stuff_int_list it can deal with variables and it also has better error reporting.
int stuff_loadout_list (int *ilp, int max_ints, int lookup_type)
{
	int count = 0; 
	int index, sexp_variable_index, variable_found;
	char str[128];

	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading loadout list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();

	while (*Mp != ')') {
		if (count >= max_ints) {
			Error(LOCATION, "Loadout contains too many entries.\n");
		}

		index = -1;
		sexp_variable_index = NOT_SET_BY_SEXP_VARIABLE;
		variable_found = get_string_or_variable (str); 

		// if we've got a variable get the variable index and copy it's value into str so that regardless of whether we found 
		// a variable or not it now holds the name of the ship or weapon we're interested in.
		if (variable_found) {
			Assert (lookup_type != CAMPAIGN_LOADOUT_SHIP_LIST );
			sexp_variable_index = get_index_sexp_variable_name(str);
			strcpy_s (str, Sexp_variables[sexp_variable_index].text);
		}

		switch (lookup_type) {
			case MISSION_LOADOUT_SHIP_LIST:
			case CAMPAIGN_LOADOUT_SHIP_LIST:
				index = ship_info_lookup(str);
				break;

			case MISSION_LOADOUT_WEAPON_LIST:
			case CAMPAIGN_LOADOUT_WEAPON_LIST:
				index = weapon_info_lookup(str);
				break;

			default:
				Int3();
		}

		// Complain if this isn't a valid ship or weapon and we are loading a mission. Campaign files can be loading containing 
		// no ships from the current tables (when swapping mods) so don't report that as an error. 
		if (index < 0 && (lookup_type == MISSION_LOADOUT_SHIP_LIST || lookup_type == MISSION_LOADOUT_WEAPON_LIST)) {
			// print a warning in debug mode
			Warning(LOCATION, "Invalid type \"%s\" found in loadout of mission file...skipping", str);
			// increment counter for release FRED builds. 
			Num_unknown_loadout_classes++;

			clean_loadout_list_entry(); 
			continue;
		}

		// similarly, complain if this is a valid ship or weapon class that the player can't use
		if ((lookup_type == MISSION_LOADOUT_SHIP_LIST) && (!(Ship_info[index].flags & SIF_PLAYER_SHIP)) ) {
			clean_loadout_list_entry(); 
			Warning(LOCATION, "Ship type \"%s\" found in loadout of mission file. This class is not marked as a player ship...skipping", str);
			continue;
		}
		else if ((lookup_type == MISSION_LOADOUT_WEAPON_LIST) && (!(Weapon_info[index].wi_flags & WIF_PLAYER_ALLOWED)) ) {
			clean_loadout_list_entry(); 
			nprintf(("Warning",  "Warning: Weapon type %s found in loadout of mission file. This class is not marked as a player allowed weapon...skipping\n", str));
			if ( !Is_standalone )
				Warning(LOCATION, "Weapon type \"%s\" found in loadout of mission file. This class is not marked as a player allowed weapon...skipping", str);
			continue;
		}
		
		// we've found a real item. Add its index to the list.
		if (count < max_ints) {
			ilp[count++] = index;
		}
		
		ignore_white_space();

		// Campaign lists need go no further
		if (lookup_type == CAMPAIGN_LOADOUT_SHIP_LIST || lookup_type == CAMPAIGN_LOADOUT_WEAPON_LIST) {
			continue;
		}
		
		// record the index of the variable that gave us this item if any
		if (count < max_ints) {
			ilp[count++] = sexp_variable_index;
		}

		// Now read in the number of this type available. The number must be positive
		count = stuff_int_or_variable(ilp, count, true);

		ignore_white_space();
	}

	Mp++;
	return count;
}

//Stuffs an integer list like stuff_int_list.
int stuff_float_list(float* flp, int max_floats)
{
	int count = 0;
	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading float list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();
	while(*Mp != ')')
	{
		Assert(count < max_floats);
		if (count < max_floats) {
			stuff_float(&flp[count++]);
		} else {
			float dummy;
			stuff_float(&dummy);
		}
		ignore_white_space();
	}

	Mp++;

	return count;
}

//	Marks an integer list.
//	This is of the form ( i* )
//	  where i is an integer.
//	If a specified string is found in the lookup and its value is 7, then the 7th value
//	in the array is set.
void mark_int_list(int *ilp, int max_ints, int lookup_type)
{
	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Marking integer list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();

	while (*Mp != ')') {
		if (*Mp == '"') {
			int num = 0;
			char str[128];

			get_string(str);
			switch(lookup_type) {
				case SHIP_TYPE:
					num = ship_name_lookup(str);	// returns index of Ship[] entry with name
					break;
			
				case SHIP_INFO_TYPE:
					num = ship_info_lookup(str);	// returns index of Ship_info[] entry with name
					break;
	
				case WEAPON_LIST_TYPE:
					num = weapon_info_lookup(str);
					break;

				default:
					Error(LOCATION,"Unknown lookup_type in stuff_int_list");
					break;
			}

			if ( (num < 0) || (num >= max_ints) )
				Error(LOCATION, "Unable to find string \"%s\" in mark_int_list.\n", str);

//			ilp[num] = 1;
		
		} else {
			int	tval;

			stuff_int(&tval);
			Assert((tval >= 0) && (tval < max_ints));
			if (tval >= 0 && tval < max_ints) {
				ilp[tval] = 1;
			}
		}
		
		ignore_white_space();
	}

	Mp++;

}


//	Stuff a vec3d struct, which is 3 floats.
void stuff_vec3d(vec3d *vp)
{
	stuff_float(&vp->xyz.x);
	stuff_float(&vp->xyz.y);
	stuff_float(&vp->xyz.z);
}

void stuff_parenthesized_vec3d(vec3d *vp)
{
	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading parenthesized vec3d.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 11);
	} else {
		Mp++;
		stuff_vec3d(vp);
		ignore_white_space();
		if (*Mp != ')') {
			error_display(1, "Reading parenthesized vec3d.  Found [%c].  Expecting ')'.\n", *Mp);
			longjmp(parse_abort, 12);
		}
		Mp++;
	}

}

//	Stuffs vec3d list.  *vlp is an array of vec3ds.
//	This is of the form ( (vec3d)* )
//	  (where * is a kleene star, not a pointer indirection)
// For example, ( (1 2 3) (2 3 4) (2 3 5) )
//		 is a list of three vec3ds.
int stuff_vec3d_list(vec3d *vlp, int max_vecs)
{
	int	count = 0;

	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading vec3d list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;

	ignore_white_space();

	while (*Mp != ')') {
		Assert(count < max_vecs);
		if (count < max_vecs) {
			stuff_parenthesized_vec3d(&vlp[count++]);
		} else {
			vec3d temp;
			stuff_parenthesized_vec3d(&temp);
		}
		
		ignore_white_space();
	}

	Mp++;

	return count;
}

// ditto the above, but a vector of vec3ds...
int stuff_vec3d_list(SCP_vector<vec3d> &vec_list)
{
	ignore_white_space();

	if (*Mp != '(') {
		error_display(1, "Reading vec3d list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;

	ignore_white_space();

	while (*Mp != ')') {
		vec3d temp;
		stuff_parenthesized_vec3d(&temp);
		vec_list.push_back(temp);
		
		ignore_white_space();
	}

	Mp++;

	return vec_list.size();
}

//	Stuff a matrix, which is 3 vec3ds.
void stuff_matrix(matrix *mp)
{
	stuff_vec3d(&mp->vec.rvec);
	stuff_vec3d(&mp->vec.uvec);
	stuff_vec3d(&mp->vec.fvec);
}


//	Given a string, find it in a string array.
//	*description is only used for diagnostics in case it can't be found.
//	*str1 is the string to be found.
//	*strlist is the list of strings to search.
//	max is the number of entries in *strlist to scan.
int string_lookup(char *str1, char *strlist[], int max, char *description, int say_errors)
{
	int	i;

	for (i=0; i<max; i++) {
		Assert(strlen(strlist[i]) != 0); //-V805

		if (!stricmp(str1, strlist[i]))
			return i;
	}

	if (say_errors)
		error_display(0, "Unable to find [%s] in %s list.\n", str1, description);

	return -1;
}

//	Find a required string (*id), then stuff the text of type f_type that
// follows it at *addr.  *strlist[] contains the strings it should try to
// match.
void find_and_stuff(char *id, int *addr, int f_type, char *strlist[], int max, char *description)
{
	char	token[128];
	int checking_ship_classes = (stricmp(id, "$class:") == 0);

	// Goober5000 - don't say errors when we're checking classes because 1) we have more checking to do; and 2) we will say a redundant error later
	required_string(id);
	stuff_string(token, f_type, sizeof(token));
	*addr = string_lookup(token, strlist, max, description, !checking_ship_classes);

	// Goober5000 - handle certain FSPort idiosyncracies with ship classes
	if (*addr < 0 && checking_ship_classes)
	{
		int idx = ship_info_lookup(token);
		
		if (idx >= 0)
			*addr = string_lookup(Ship_info[idx].name, strlist, max, description, 0);
		else
			*addr = -1;
	}
}

void find_and_stuff_optional(char *id, int *addr, int f_type, char *strlist[], int max, char *description)
{
	char token[128];
	
	if(optional_string(id))
	{
		stuff_string(token, f_type, sizeof(token));
		*addr = string_lookup(token, strlist, max, description, 1);
	}
}

//	Mp points at a string.
//	Find the string in the list of strings *strlist[].
// Returns the index of the match, -1 if none.
int match_and_stuff(int f_type, char *strlist[], int max, char *description)
{
	char	token[128];

	stuff_string(token, f_type, sizeof(token));
	return string_lookup(token, strlist, max, description, 0);
}

void find_and_stuff_or_add(char *id, int *addr, int f_type, char *strlist[], int *total,
	int max, char *description)
{
	char	token[128];

	*addr = -1;
	required_string(id);
	stuff_string(token, f_type, sizeof(token));
	if (*total)
		*addr = string_lookup(token, strlist, *total, description, 0);

	if (*addr == -1)  // not in list, so lets try and add it.
	{
		Assert(*total < max);
		strcpy(strlist[*total], token);
		*addr = (*total)++;
	}
}

// pause current parsing so that some else can be parsed without interferring
// with the currently parsing file
void pause_parse()
{
	Assert( !Parsing_paused );

	Mp_save = Mp;

	Warning_count_save = Warning_count;
	Error_count_save = Error_count;

	strcpy_s(parse_error_text_save, parse_error_text);
	strcpy_s(Current_filename_save, Current_filename);

	Parsing_paused = 1;	
}

// unpause parsing to continue with previously parsing file
void unpause_parse()
{
	Assert( Parsing_paused );

	if (!Parsing_paused)
		return;

	Mp = Mp_save;

	Warning_count = Warning_count_save;
	Error_count = Error_count_save;

	strcpy_s(parse_error_text, parse_error_text_save);
	strcpy_s(Current_filename, Current_filename_save);

	Parsing_paused = 0;
}

void reset_parse(char *text)
{
	if (text != NULL) {
		Mp = text;
	} else {
		Mp = Mission_text;
	}

	Warning_count = 0;
	Error_count = 0;

	strcpy_s(parse_error_text, "");//better error mesages-Bobboau

	strcpy_s(Current_filename, Current_filename_sub);
}

// Display number of warnings and errors at the end of a parse.
void display_parse_diagnostics()
{
	nprintf(("Parse", "\nParse complete.\n"));
	nprintf(("Parse", "%i errors.  %i warnings.\n", Error_count, Warning_count));
}

// Splits a string into 2 lines if the string is wider than max_pixel_w pixels.  A null
// terminator is placed where required to make the first line <= max_pixel_w.  The remaining
// text is returned (leading whitespace removed).  If the line doesn't need to be split,
// NULL is returned.
char *split_str_once(char *src, int max_pixel_w)
{
	char *brk = NULL;
	int i, w, len, last_was_white = 0;

	Assert(src);
	Assert(max_pixel_w > 0);
	
	gr_get_string_size(&w, NULL, src);
	if ( (w <= max_pixel_w) && !strstr(src, "\n") ) {
		return NULL;  // string doesn't require a cut
	}

	len = strlen(src);
	for (i=0; i<len; i++) {
		gr_get_string_size(&w, NULL, src, i);
		if ( w > max_pixel_w )
			break;

		if (src[i] == '\n') {  // reached natural end of line
			src[i] = 0;
			return src + i + 1;
		}

		if (is_white_space(src[i])) {
			if (!last_was_white)
				brk = src + i;

			last_was_white = 1;

		} else {
			last_was_white = 0;
		}
	}

	// if we are over max pixel width and weren't able to come up with a good non-word
	// split then just return the original src text and the calling function should
	// have to handle the result
	if ( (w > max_pixel_w) && ((i == 0) || !brk) ) {
		return src;
	}

	if (!brk) {
		brk = src + i;
	}

	*brk = 0;
	src = brk + 1;
	while (is_white_space(*src))
		src++;

	if (!*src)
		return NULL;  // end of the string anyway
		
	if (*src == '\n')
		src++;

	return src;
}

#define SPLIT_STR_BUFFER_SIZE	512

// --------------------------------------------------------------------------------------
// split_str() 
//
// A general function that will split a string into several lines.  Lines are allowed up
// to max_pixel_w pixels.  Breaks are found in white space.
//
// Supports \n's in the strings!
//
// parameters:		src			=>		source string to be broken up
//						max_pixel_w	=>		max width of line in pixels
//						n_chars		=>		output array that will hold number of characters in each line
//						p_str			=>		output array of pointers to start of lines within src
//						max_lines	=>		limit of number of lines to break src up into
//						ignore_char	=>		OPTIONAL parameter (default val -1).  Ignore words starting with this character
//												This is useful when you want to ignore embedded control information that starts
//												with a specific character, like $ or #
//
//	returns:			number of lines src is broken into
//						-1 is returned when an error occurs
//
int split_str(const char *src, int max_pixel_w, int *n_chars, const char **p_str, int max_lines, char ignore_char)
{
	char buffer[SPLIT_STR_BUFFER_SIZE];
	const char *breakpoint = NULL;
	int sw, new_line = 1, line_num = 0, last_was_white = 0;
	int ignore_until_whitespace, buf_index;
	
	// check our assumptions..
	Assert(src != NULL);
	Assert(n_chars != NULL);
	Assert(p_str != NULL);
	Assert(max_lines > 0);
	Assert(max_pixel_w > 0);
	
	memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);
	buf_index = 0;
	ignore_until_whitespace = 0;

	// get rid of any leading whitespace
	while (is_white_space(*src))
		src++;

	new_line = 1;
	p_str[0] = NULL;

	// iterate through chars in line, keeping track of most recent "white space" location that can be used
	// as a line splitting point if necessary
	for (; *src; src++) {
		if (line_num >= max_lines)
			return line_num;  // time to bail out

		// starting a new line of text, init stuff for that
		if (new_line) {
			p_str[line_num] = NULL;
			if (is_gray_space(*src))
				continue;

			p_str[line_num] = src;
			breakpoint = NULL;
			new_line = 0;
		}

		// maybe skip leading whitespace
		if (ignore_until_whitespace) {
			if ( is_white_space(*src) )
				ignore_until_whitespace = 0;

			continue;
		}

		// if we have a newline, split the line here
		if (*src == '\n') {
			n_chars[line_num] = src - p_str[line_num];  // track length of line
			line_num++;
			if (line_num < max_lines) {
				p_str[line_num] = NULL;
			}
			new_line = 1;

			memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);
			buf_index = 0;
			continue;
		}

		if (*src == ignore_char) {
			ignore_until_whitespace = 1;
			continue;
		}

		if (is_gray_space(*src)) {
			if (!last_was_white)  // track at first whitespace in a series of whitespace
				breakpoint = src;

			last_was_white = 1;

		} else {
			// indicate next time around that this wasn't a whitespace character
			last_was_white = 0;
		}

		// throw it in our buffer
		buffer[buf_index] = *src;
		buf_index++;
		buffer[buf_index] = 0;  // null terminate it
	
		gr_get_string_size(&sw, NULL, buffer);
		if (sw >= max_pixel_w) {
			const char *end;

			if (breakpoint) {
				end = src = breakpoint;

			} else {
				end = src;  // force a split here since to whitespace
				src--;  // reuse this character in next line
			}

			n_chars[line_num] = end - p_str[line_num];  // track length of line
			Assert(n_chars[line_num]);
			line_num++;
			if (line_num < max_lines) {
				p_str[line_num] = NULL;
			}
			new_line = 1;

			memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);
			buf_index = 0;
			continue;
		}
	}	// end for

	if (!new_line && p_str[line_num]) {
		n_chars[line_num] = src - p_str[line_num];  // track length of line
		Assert(n_chars[line_num]);
		line_num++;
	}

	return line_num;
}

int split_str(const char *src, int max_pixel_w, SCP_vector<int> &n_chars, SCP_vector<const char*> &p_str, char ignore_char)
{
	char buffer[SPLIT_STR_BUFFER_SIZE];
	const char *breakpoint = NULL;
	int sw, new_line = 1, line_num = 0, last_was_white = 0;
	int ignore_until_whitespace = 0, buf_index = 0;
	
	// check our assumptions..
	Assert(src != NULL);
	Assert(max_pixel_w > 0);
	
	memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);

	// get rid of any leading whitespace
	while (is_white_space(*src))
		src++;

	p_str.clear();

	// iterate through chars in line, keeping track of most recent "white space" location that can be used
	// as a line splitting point if necessary
	for (; *src; src++) {

		// starting a new line of text, init stuff for that
		if (new_line) {
			if (is_gray_space(*src))
				continue;

			p_str.push_back(src);
			breakpoint = NULL;
			new_line = 0;
		}

		// maybe skip leading whitespace
		if (ignore_until_whitespace) {
			if ( is_white_space(*src) ) {
				ignore_until_whitespace = 0;

				// don't eat the newline
				if (*src == EOLN)
					src--;
			}

			continue;
		}

		// if we have a newline, split the line here
		if (*src == '\n') {
			n_chars.push_back(src - p_str.at(line_num));  // track length of line
			line_num++;
			new_line = 1;

			memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);
			buf_index = 0;
			continue;
		}

		if (*src == ignore_char) {
			ignore_until_whitespace = 1;
			continue;
		}

		if (is_gray_space(*src)) {
			if (!last_was_white)  // track at first whitespace in a series of whitespace
				breakpoint = src;

			last_was_white = 1;

		} else {
			// indicate next time around that this wasn't a whitespace character
			last_was_white = 0;
		}

		// throw it in our buffer
		buffer[buf_index] = *src;
		buf_index++;
		buffer[buf_index] = 0;  // null terminate it
	
		gr_get_string_size(&sw, NULL, buffer);
		if (sw >= max_pixel_w) {
			const char *end;

			if (breakpoint) {
				end = src = breakpoint;

			} else {
				end = src;  // force a split here since to whitespace
				src--;  // reuse this character in next line
			}

			n_chars.push_back(end - p_str.at(line_num));  // track length of line
			Assert(n_chars.at(line_num));
			line_num++;
			new_line = 1;

			memset(buffer, 0, SPLIT_STR_BUFFER_SIZE);
			buf_index = 0;
			continue;
		}
	}	// end for

	if (!new_line && p_str.at(line_num)) {
		n_chars.push_back(src - p_str.at(line_num));  // track length of line
		Assert(n_chars.at(line_num));
		line_num++;
	}

	return line_num;
}

// Goober5000
// accounts for the dumb communications != communication, etc.
int subsystem_stricmp(const char *str1, const char *str2)
{
	Assert(str1 && str2);

	// ensure len-1 will be valid
	if (!*str1 || !*str2)
		return stricmp(str1, str2);

	// calc lengths
	int len1 = strlen(str1);
	int len2 = strlen(str2);

	// get rid of trailing s on s1?
	if (tolower(*(str1+len1-1) == 's'))
		len1--;

	// get rid of trailing s on s2?
	if (tolower(*(str2+len2-1) == 's'))
		len2--;

	// once we remove the trailing s on both names, they should be the same length
	if (len1 > len2)
		return 1;
	if (len1 < len2)
		return -1;

	// now do the comparison
	return strnicmp(str1, str2, len1);
}

// Goober5000
// current algorithm adapted from http://www.codeproject.com/string/stringsearch.asp
char *stristr(const char *str, const char *substr)
{
	// check for null and insanity
	Assert(str);
	Assert(substr);
	if (str == NULL || substr == NULL || *substr == '\0')
		return NULL;

	// save both a lowercase and an uppercase version of the first character of substr
	char substr_ch_lower = (char)tolower(*substr);
	char substr_ch_upper = (char)toupper(*substr);

	// find the maximum distance to search
	char *upper_bound = (char *)str + strlen(str) - strlen(substr);

	// loop through every character of str
	for (char *start = (char *)str; start <= upper_bound; start++)
	{
		// check first character of substr
		if ((*start == substr_ch_upper) || (*start == substr_ch_lower))
		{
			// first character matched, so check the rest
			for (char *str_ch = start+1, *substr_ch = (char *)substr+1; *substr_ch != '\0'; str_ch++, substr_ch++)
			{
				// character match?
				if (*str_ch == *substr_ch)
					continue;

				// converted character match?
				if (tolower(*str_ch) == tolower(*substr_ch))
					continue;

				// mismatch
				goto stristr_continue_outer_loop;
			}

			// finished inner loop with success!
			return start;
		}

stristr_continue_outer_loop:
		/* NO-OP */ ;
	}

	// no match
	return NULL;
}

// Goober5000
bool can_construe_as_integer(const char *text)
{
	// trivial case; evaluates to 0
	if (*text == '\0')
		return true;

	// number sign or digit for first char
	if ((*text != '+') && (*text != '-') && !isdigit(*text))
		return false;

	// check digits for rest
	for (const char *p = text + 1; *p != '\0'; p++)
	{
		if (!isdigit(*p))
			return false;
	}

	return true;
}

// Goober5000
// yoinked gratefully from dbugfile.cpp
void vsprintf(SCP_string &dest, const char *format, va_list ap)
{
	const int MAX_BUF = 64;
	const char *handled_types = "diouxXcfsn%";

	int buf_src_len;
	char buf_src[MAX_BUF];
	char buf_dest[MAX_BUF];

	const char *p;
	int *pint;
	long ival;
	double dval;

	// clear string
	dest = "";

	// Add each extra parameter to string
	for (p = format; *p; ++p)
	{
		if (*p != '%')
		{
			dest += *p;
			continue;
		}

		// find the specifier that comes next
		buf_src[0] = '%';
		buf_src_len = 1;
		do {
			++p;
			if (!*p || (buf_src_len >= MAX_BUF))
			{
				Warning(LOCATION, "Could not find a sprintf specifier within %d characters for format '%s', pos %d!", MAX_BUF, format, (p - format));

				// unsafe to continue handling this va_list
				dest += buf_src;
				return;
			}

			buf_src[buf_src_len] = *p;
			buf_src_len++;
		} while (strchr(handled_types, *p) == NULL);
		buf_src[buf_src_len] = 0;

		// handle it
		switch (*p)
		{
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			{
				ival = va_arg(ap, int);
				sprintf(buf_dest, buf_src, ival);
				dest += buf_dest;
				break;
			}
			case 'c':
			{
				dest += (char) va_arg(ap, char);
				break;
			}
			case 'f':
			{
				dval = va_arg(ap, double);
				sprintf(buf_dest, buf_src, dval);
				dest += buf_dest;
				break;
			}
			case 's':
			{
				dest += va_arg(ap, char *);
				break;
			}
			case 'n':
			{
				pint = va_arg(ap, int *);
				Assert(pint != NULL);
				*pint = dest.length();
			}
			case '%':
			{
				dest += '%';	// escaped %
				break;
			}
			default:
			{
				sprintf(buf_dest, "N/A: %%%c", *p);
				dest += buf_dest;
				break;
			}
		}
	}
}

void sprintf(SCP_string &dest, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(dest, format, args);
	va_end(args);
}

// Goober5000
bool end_string_at_first_hash_symbol(char *src)
{
	char *p;
	Assert(src);

	p = get_pointer_to_first_hash_symbol(src);
	if (p)
	{
		while (*(p-1) == ' ')
			p--;

		*p = '\0';
		return true;
	}

	return false;
}

// Goober5000
bool end_string_at_first_hash_symbol(SCP_string &src)
{
	int index = get_index_of_first_hash_symbol(src);
	if (index >= 0)
	{
		while (index > 0 && src[index-1] == ' ')
			index--;

		src.resize(index);
		return true;
	}

	return false;
}

// Goober5000
char *get_pointer_to_first_hash_symbol(char *src)
{
	Assert(src);
	return strchr(src, '#');
}

// Goober5000
int get_index_of_first_hash_symbol(SCP_string &src)
{
	size_t pos = src.find('#');
	return (pos == SCP_string::npos) ? -1 : pos;
}

// Goober5000
int replace_one(char *str, char *oldstr, char *newstr, uint max_len, int range)
{
	Assert(str && oldstr && newstr);

	// search
	char *ch = stristr(str, oldstr);

	// found?
	if (ch)
	{
		// not found within bounds?
		if ((range > 0) && ((ch - str) > range))
		{
			return 0;
		}

		// determine if replacement will exceed max len
		if (strlen(str) + strlen(newstr) - strlen(oldstr) > max_len)
		{
			return -1;
		}

		// allocate temp string to hold extra stuff
		char *temp = (char *) vm_malloc(sizeof(char) * max_len);

		// ensure allocation was successful
		if (temp)
		{
			// save remainder of string
			strcpy_s(temp, sizeof(char)*max_len, ch + strlen(oldstr));

			// replace
			strcpy(ch, newstr);

			// append rest of string
			strcpy(ch + strlen(newstr), temp);
		}

		// free temp string
		vm_free(temp);
	}
	// not found
	else
	{
		return 0;
	}

	// return pos of replacement
	return (ch - str);
}

// Goober5000
int replace_all(char *str, char *oldstr, char *newstr, uint max_len, int range)
{
	int val, tally = 0;

	while ((val = replace_one(str, oldstr, newstr, max_len, range)) > 0)
	{
		tally++;

		// adjust range (if we have one), because the text length might have changed
		if (range) {
			range += strlen(newstr) - strlen(oldstr);
		}
	}

	return (val < 0) ? val : tally;
}

SCP_string& replace_one(SCP_string& context, const SCP_string& from, const SCP_string& to)
{
	size_t foundHere;
	if ((foundHere = context.find(from, 0)) != SCP_string::npos)
	{
		context.replace(foundHere, from.length(), to);
	}
	return context;
}

SCP_string& replace_one(SCP_string& context, const char* from, const char* to)
{
	size_t foundHere;
	if ((foundHere = context.find(from, 0)) != SCP_string::npos)
	{
		context.replace(foundHere, strlen(from), to);
	}
	return context;
}

// http://www.cppreference.com/wiki/string/replace
SCP_string& replace_all(SCP_string& context, const SCP_string& from, const SCP_string& to)
{
	size_t from_len = from.length();
	size_t to_len = to.length();

	size_t lookHere = 0;
	size_t foundHere;
	while ((foundHere = context.find(from, lookHere)) != SCP_string::npos)
	{
		context.replace(foundHere, from_len, to);
		lookHere = foundHere + to_len;
	}
	return context;
}

// http://www.cppreference.com/wiki/string/replace
SCP_string& replace_all(SCP_string& context, const char* from, const char* to)
{
	size_t from_len = strlen(from);
	size_t to_len = strlen(to);

	size_t lookHere = 0;
	size_t foundHere;
	while ((foundHere = context.find(from, lookHere)) != SCP_string::npos)
	{
		context.replace(foundHere, from_len, to);
		lookHere = foundHere + to_len;
	}
	return context;
}

// WMC
// Compares two strings, ignoring (last) extension
// Returns 0 if equal, nonzero if not
int strextcmp(const char *s1, const char *s2)
{
	// sanity check
	Assert( (s1 != NULL) && (s2 != NULL) );

	// find last '.' in both strings
	char *s1_end = (char *)strrchr(s1, '.');
	char *s2_end = (char *)strrchr(s2, '.');

	// get length
	size_t s1_len, s2_len;

	if (s1_end != NULL)
		s1_len = (s1_end - s1);
	else
		s1_len = strlen(s1);

	if (s2_end != NULL)
		s2_len = (s2_end - s2);
	else
		s2_len = strlen(s2);

	// if the lengths aren't the same then it's deffinitely not the same name
	if (s2_len != s1_len)
		return 1;

	return strnicmp(s1, s2, s1_len);
}

//WMC
void backspace(char* src)
{
	Assert(src!= NULL);		//this would be bad

	char *dest = src;
	src++;

	while(*src != '\0') {
		*dest++ = *src++;
	}

	*dest = '\0';
}

// Goober5000
void format_integer_with_commas(char *buf, int integer, bool use_comma_with_four_digits)
{
	int old_pos, new_pos, triad_count;
	char backward_buf[32];

	// print an initial string of just the digits
	sprintf(buf, "%d", integer);

	// no commas needed?
	if ((integer < 1000) || (integer < 10000 && !use_comma_with_four_digits))
		return;

	// scan the string backwards, writing commas after every third digit
	new_pos = 0;
	triad_count = 0;
	for (old_pos = strlen(buf) - 1; old_pos >= 0; old_pos--)
	{
		backward_buf[new_pos] = buf[old_pos];
		new_pos++;
		triad_count++;

		if (triad_count == 3 && old_pos > 0)
		{
			backward_buf[new_pos] = ',';
			new_pos++;
			triad_count = 0;
		}
	}
	backward_buf[new_pos] = '\0';

	// now reverse the string
	new_pos = 0;
	for (old_pos = strlen(backward_buf) - 1; old_pos >= 0; old_pos--)
	{
		buf[new_pos] = backward_buf[old_pos];
		new_pos++;
	}
	buf[new_pos] = '\0';
}

// Goober5000
// there's probably a better way to do this, but this way works and is clear and short
int scan_fso_version_string(const char *text, int *major, int *minor, int *build, int *revis)
{
	int val;

	val = sscanf(text, ";;FSO %i.%i.%i.%i;;", major, minor, build, revis);
	if (val == 4)
		return val;

	*revis = 0;
	val = sscanf(text, ";;FSO %i.%i.%i;;", major, minor, build);
	if (val == 3)
		return val;

	*build = *revis = 0;
	val = sscanf(text, ";;FSO %i.%i;;", major, minor);
	if (val == 2)
		return val;

	*minor = *build = *revis = 0;
	val = sscanf(text, ";;FSO %i;;", major);
	if (val == 1)
		return val;

	*major = *minor = *build = *revis = 0;
	return 0;
}

// Goober5000 - used for long Warnings, Errors, and FRED error messages with SEXPs
void truncate_message_lines(SCP_string &text, int num_allowed_lines)
{
	Assert(num_allowed_lines > 0);
	size_t find_from = 0;

	while (find_from < text.size())
	{
		if (num_allowed_lines <= 0)
		{
			text.resize(find_from);
			text.append("[...]");
			break;
		}

		size_t pos = text.find('\n', find_from);
		if (pos == SCP_string::npos)
			break;

		num_allowed_lines--;
		find_from = pos + 1;
	}
}

// Goober5000 - ugh, I can't see why they didn't just use stuff_*_list for these;
// the only differece is the lack of parentheses

// from aicode.cpp
// Stuff a list of floats at *plist.
void parse_float_list(float *plist, int size)
{
	int i;

	for (i=0; i<size; i++)
	{
		stuff_float(&plist[i]);
	}
}

// from aicode.cpp
// Stuff a list of ints at *plist.
void parse_int_list(int *ilist, int size)
{
	int i;

	for (i=0; i<size; i++)
	{
		stuff_int(&ilist[i]);
	}
}

// parse a modular table of type "name_check" and parse it using the specified function callback
int parse_modular_table(char *name_check, void (*parse_callback)(char *filename), int path_type, int sort_type)
{
	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];
	int i, num_files = 0;

	if ( (name_check == NULL) || (parse_callback == NULL) || ((*name_check) != '*') ) {
		Int3();
		return 0;
	}

	num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, path_type, name_check, sort_type);

	Parsing_modular_table = true;

	for (i = 0; i < num_files; i++){
		strcat(tbl_file_names[i], ".tbm");
		mprintf(("TBM  =>  Starting parse of '%s' ...\n", tbl_file_names[i]));
		(*parse_callback)(tbl_file_names[i]);
	}

	Parsing_modular_table = false;

	return num_files;
}

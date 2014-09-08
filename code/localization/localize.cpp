/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include <ctype.h>
#include "localization/localize.h"
#include "parse/parselo.h"
#include "osapi/osregistry.h"
#include "parse/encrypt.h"
#include "playerman/player.h"
#include "cfile/cfile.h"




// ------------------------------------------------------------------------------------------------------------
// LOCALIZE DEFINES/VARS
//

// general language/localization data ---------------------

// current language
int Lcl_current_lang = FS2_OPEN_DEFAULT_LANGUAGE;
SCP_vector<lang_info> Lcl_languages;

// These are the original languages supported by FS2. The code expects these languages to be supported even if the tables don't

#define NUM_BUILTIN_LANGUAGES		4
lang_info Lcl_builtin_languages[NUM_BUILTIN_LANGUAGES] = {
	{ "English",		"",		{127,0,176,0,0},	589986744},				// English
	{ "German",			"gr",	{164,0,176,0,0},	-1132430286 },			// German
	{ "French",			"fr",	{164,0,176,0,0},	0 },					// French
	{ "Polish",			"pl",	{127,0,176,0,0},	-1131728960},			// Polish
};

int Lcl_special_chars;

// use these to replace *_BUILD, values before
// only 1 will be active at a time
int Lcl_fr = 0;
int Lcl_gr = 0;
int Lcl_pl = 0;
int Lcl_english = 1;


// executable string localization data --------------------

// XSTR_SIZE is the total count of unique XSTR index numbers.  An index is used to
// reference an entry in strings.tbl.  This is used for translation of strings from
// the english version (in the code) to a foreign version (in the table).  Thus, if you
// add a new string to the code, you must assign it a new index.  Use the number below for
// that index and increase the number below by one.
#define XSTR_SIZE	1638


// struct to allow for strings.tbl-determined x offset
// offset is 0 for english, by default
typedef struct {
	const char *str;
	int  offset_x;				// string offset in 640
	int  offset_x_hi;			// string offset in 1024
} lcl_xstr;

//char *Xstr_table[XSTR_SIZE];
lcl_xstr Xstr_table[XSTR_SIZE];
int Xstr_inited = 0;


// table/mission externalization stuff --------------------
#define PARSE_TEXT_BUF_SIZE			PARSE_BUF_SIZE
#define PARSE_ID_BUF_SIZE			5
#define LCL_MAX_STRINGS					4500
char *Lcl_ext_str[LCL_MAX_STRINGS];


// ------------------------------------------------------------------------------------------------------------
// LOCALIZE FORWARD DECLARATIONS
//

// given a valid XSTR() tag piece of text, extract the string portion, return it in out, nonzero on success
int lcl_ext_get_text(const char *xstr, char *out);
int lcl_ext_get_text(const SCP_string &xstr, SCP_string &out);

// given a valid XSTR() tag piece of text, extract the id# portion, return the value in out, nonzero on success
int lcl_ext_get_id(const char *xstr, int *out);
int lcl_ext_get_id(const SCP_string &xstr, int *out);

// if the char is a valid char for a signed integer value string
int lcl_is_valid_numeric_char(char c);

// parses the string.tbl and reports back only on the languages it found
void parse_stringstbl_quick(const char *filename);


// ------------------------------------------------------------------------------------------------------------
// LOCALIZE FUNCTIONS
//

// initialize localization, if no language is passed - use the language specified in the registry
void lcl_init(int lang_init)
{
	atexit(lcl_xstr_close);

	char lang_string[128];
	const char *ret;
	int lang, idx, i;
	int rval;

	// initialize encryption
	encrypt_init();

	// setup English
	Lcl_languages.push_back(Lcl_builtin_languages[FS2_OPEN_DEFAULT_LANGUAGE]);

	// check string.tbl to see which languages we support
	if ( (rval = setjmp(parse_abort)) != 0 ) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "strings.tbl", rval));
	}
	else {
		parse_stringstbl_quick("strings.tbl");
	}

	parse_modular_table(NOX("*-lcl.tbm"), parse_stringstbl_quick);

	// if the only language we have at this point is English, we need to setup the builtin languages as we might be dealing with an old style strings.tbl
	// which doesn't support anything beyond the builtin languages. Note, we start at i = 1 because we added English above.
	if ((int)Lcl_languages.size() == 1) {
		for (i=1; i<NUM_BUILTIN_LANGUAGES; i++) {
			Lcl_languages.push_back(Lcl_builtin_languages[i]);
		}
	}

	// read the language from the registry
	if(lang_init < 0){
		memset(lang_string, 0, 128);
		// default to DEFAULT_LANGUAGE (which should be English so we don't have to put German text 
		// in tstrings in the #default section)
		ret = os_config_read_string(NULL, "Language", Lcl_languages[FS2_OPEN_DEFAULT_LANGUAGE].lang_name);

		if(ret == NULL){
			Error(LOCATION, "Default language not found."); 
		}		

		strcpy_s(lang_string, ret);
		

		// look it up
		lang = -1;
		for(idx = 0; idx < (int)Lcl_languages.size(); idx++){
			if(!stricmp(Lcl_languages[idx].lang_name, lang_string)){
				lang = idx;
				break;
			}
		}
		if(lang < 0){
			lang = 0;
		}	
	} else {
		Assert((lang_init >= 0) && (lang_init < (int)Lcl_languages.size()));
		lang = lang_init;
	}

	// set the language (this function takes care of setting up file pointers)
	lcl_set_language(lang);
}

// determine what language we're running in, see LCL_* defines above
int lcl_get_language()
{
	return Lcl_current_lang;
}

// parses the string.tbl to see which languages are supported. Doesn't read in any strings.
void parse_stringstbl_quick(const char *filename)
{
	lang_info language;
	int lang_idx;
	int i;

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	if (optional_string("#Supported Languages")) {
		while (required_string_either("#End","$Language:")) {			
			required_string("$Language:");
			stuff_string(language.lang_name, F_RAW, LCL_LANG_NAME_LEN + 1);
			required_string("+Extension:");
			stuff_string(language.lang_ext, F_RAW, LCL_LANG_NAME_LEN + 1);
			required_string("+Special Character Index:");
			stuff_ubyte(&language.special_char_indexes[0]);
			for (i = 1; i < MAX_FONTS; ++i) {
				// default to "none"/0 except for font03 which defaults to 176
				// NOTE: fonts.tbl may override these values
				if (i == FONT3) {
					language.special_char_indexes[i] = 176;
				} else {
					language.special_char_indexes[i] = 0;
				}
			}

			lang_idx = -1;

			// see if we already have this language
			for (i = 0; i < (int)Lcl_languages.size(); i++) {
				if (!strcmp(Lcl_languages[i].lang_name, language.lang_name)) {
					strcpy_s(Lcl_languages[i].lang_ext, language.lang_ext); 
					Lcl_languages[i].special_char_indexes[0] = language.special_char_indexes[0];
					lang_idx = i;
					break;
				}
			}

			// if we have a new language, add it.
			if (lang_idx == -1) {
				Lcl_languages.push_back(language);
			}
		}
	}
}

// Unified function for loading strings.tbl and tstrings.tbl (and their modular versions).
// The "external" parameter controls which format to load: true for tstrings.tbl, false for strings.tbl
void parse_stringstbl_common(const char *filename, const bool external)
{
	char chr, buf[4096];
	char language_tag[512];
	int i, z, index;
	char *p_offset = NULL;
	int offset_lo = 0, offset_hi = 0;

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	// move down to the proper section		
	memset(language_tag, 0, sizeof(language_tag));
	strcpy_s(language_tag, "#");
	if (external && Lcl_current_lang == FS2_OPEN_DEFAULT_LANGUAGE){
		strcat_s(language_tag, "default");
	} else {
		strcat_s(language_tag, Lcl_languages[Lcl_current_lang].lang_name);
	}

	if ( skip_to_string(language_tag) != 1 ) {
		mprintf(("Current language not found in %s\n", filename));
		return;
	}

	// parse all the strings in this section of the table
	while ( !check_for_string("#") ) {
		int num_offsets_on_this_line = 0;

		stuff_int(&index);
		if (external) {
			ignore_white_space();
			get_string(buf, sizeof(buf));
			drop_trailing_white_space(buf);
		} else {
			stuff_string(buf, F_RAW, sizeof(buf));
		}

		if (external && (index < 0 || index >= LCL_MAX_STRINGS)) {
			error_display(0, "Invalid tstrings table index specified (%i). Please increment LCL_MAX_STRINGS in localize.cpp.", index);
			return;
		} else if (!external && (index < 0 || index >= XSTR_SIZE)) {
			Error(LOCATION, "Invalid strings table index specified (%i)", index);
		}
		
		if (!external) {
			i = strlen(buf);

			while (i--) {
				if ( !isspace(buf[i]) )
					break;
			}

			// trim unnecessary end of string
			if (i >= 0) {
				// Assert(buf[i] == '"');
				if (buf[i] != '"') {
					// probably an offset on this entry

					// drop down a null terminator (prolly unnecessary)
					buf[i+1] = 0;

					// back up over the potential offset
					while ( !is_white_space(buf[i]) )
						i--;

					// now back up over intervening spaces
					while ( is_white_space(buf[i]) )
						i--;

					num_offsets_on_this_line = 1;

					if (buf[i] != '"') {
						// could have a 2nd offset value (one for 640, one for 1024)
						// so back up again
						while ( !is_white_space(buf[i]) )
							i--;

						// now back up over intervening spaces
						while ( is_white_space(buf[i]) )
							i--;

						num_offsets_on_this_line = 2;
					}

					p_offset = &buf[i+1];			// get ptr to string section with offset in it

					if (buf[i] != '"')
						Error(LOCATION, "%s is corrupt", filename);		// now its an error
				}

				buf[i] = 0;
			}

			// copy string into buf
			z = 0;
			for (i = 1; buf[i]; i++) {
				chr = buf[i];

				if (chr == '\\') {
					chr = buf[++i];

					if (chr == 'n')
						chr = '\n';
					else if (chr == 'r')
						chr = '\r';
				}

				buf[z++] = chr;
			}

			// null terminator on buf
			buf[z] = 0;
		}

		// write into Xstr_table (for strings.tbl) or Lcl_ext_str (for tstrings.tbl)
		if (Parsing_modular_table) {
			if ( external && (Lcl_ext_str[index] != NULL) ) {
				vm_free((void *) Lcl_ext_str[index]);
				Lcl_ext_str[index] = NULL;
			} else if ( !external && (Xstr_table[index].str != NULL) ) {
				vm_free((void *) Xstr_table[index].str);
				Xstr_table[index].str = NULL;
			}
		}

		if (external && (Lcl_ext_str[index] != NULL)) {
			Warning(LOCATION, "Tstrings table index %d used more than once", index);
		} else if (!external && (Xstr_table[index].str != NULL)) {
			Warning(LOCATION, "Strings table index %d used more than once", index);
		}

		if (external) {
			Lcl_ext_str[index] = vm_strdup(buf);
		} else {
			Xstr_table[index].str = vm_strdup(buf);
		}

		// the rest of this loop applies only to strings.tbl,
		// so we can move on to the next line if we're reading from tstrings.tbl
		if (external) {
			continue;
		}

		// read offset information, assume 0 if nonexistant
		if (p_offset != NULL) {
			if (sscanf(p_offset, "%d%d", &offset_lo, &offset_hi) < num_offsets_on_this_line) {
				// whatever is in the file ain't a proper offset
				Error(LOCATION, "%s is corrupt", filename);
			}
		}

		Xstr_table[index].offset_x = offset_lo;

		if (num_offsets_on_this_line == 2)
			Xstr_table[index].offset_x_hi = offset_hi;
		else
			Xstr_table[index].offset_x_hi = offset_lo;

		// clear out our vars
		p_offset = NULL;
		offset_lo = 0;
		offset_hi = 0;
	}
}

void parse_stringstbl(const char *filename)
{
	parse_stringstbl_common(filename, false);
}

void parse_tstringstbl(const char *filename)
{
	parse_stringstbl_common(filename, true);
}

// initialize the xstr table
void lcl_xstr_init()
{
	int i, rval;


	for (i = 0; i < XSTR_SIZE; i++)
		Xstr_table[i].str = NULL;

	for (i = 0; i < LCL_MAX_STRINGS; i++)
		Lcl_ext_str[i] = NULL;


	if ( (rval = setjmp(parse_abort)) != 0 )
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "strings.tbl", rval));
	else
		parse_stringstbl("strings.tbl");

	parse_modular_table(NOX("*-lcl.tbm"), parse_stringstbl);


	if ( (rval = setjmp(parse_abort)) != 0 )
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "tstrings.tbl", rval));
	else
		parse_tstringstbl("tstrings.tbl");

	parse_modular_table(NOX("*-tlc.tbm"), parse_tstringstbl);


	Xstr_inited = 1;
}


// free Xstr table
void lcl_xstr_close()
{
	int i;

	for (i=0; i<XSTR_SIZE; i++){
		if (Xstr_table[i].str != NULL) {
			vm_free((void *) Xstr_table[i].str);
			Xstr_table[i].str = NULL;
		}
	}

	for (i=0; i<LCL_MAX_STRINGS; i++){
		if (Lcl_ext_str[i] != NULL) {
			vm_free((void *) Lcl_ext_str[i]);
			Lcl_ext_str[i] = NULL;
		}
	}
}


// set our current language
void lcl_set_language(int lang)
{
	Lcl_current_lang = lang;

	nprintf(("General", "Setting language to %s\n", Lcl_languages[lang].lang_name));

	Assertion((Lcl_current_lang >= 0) && (Lcl_current_lang < (int)Lcl_languages.size()), "Attempt to set language to an invalid language");

	// flag the proper language as being active
	Lcl_special_chars = Lcl_languages[Lcl_current_lang].special_char_indexes[0];
	Lcl_fr = 0;
	Lcl_gr = 0;
	Lcl_pl = 0;
	Lcl_english = 0;
	if (!strcmp(Lcl_languages[Lcl_current_lang].lang_name, Lcl_builtin_languages[LCL_ENGLISH].lang_name)) {
		Lcl_english = 1;
	} else if (!strcmp(Lcl_languages[Lcl_current_lang].lang_name, Lcl_builtin_languages[LCL_FRENCH].lang_name)) {
		Lcl_fr = 1;
	} else if (!strcmp(Lcl_languages[Lcl_current_lang].lang_name, Lcl_builtin_languages[LCL_GERMAN].lang_name)) {
		Lcl_gr = 1;
	} else if (!strcmp(Lcl_languages[Lcl_current_lang].lang_name, Lcl_builtin_languages[LCL_POLISH].lang_name)) {
		Lcl_pl = 1;
	}
}

ubyte lcl_get_font_index(int font_num)
{
	Assertion((font_num >= 0) && (font_num < MAX_FONTS), "Passed an invalid font index");
	Assertion((Lcl_current_lang >= 0) && (Lcl_current_lang < (int)Lcl_languages.size()), "Current language is not valid, can't get font indexes");

	return Lcl_languages[Lcl_current_lang].special_char_indexes[font_num];
}

// maybe add on an appropriate subdirectory when opening a localized file
void lcl_add_dir(char *current_path)
{
	char last_char;
	int path_len;

	// if the disk extension is 0 length, don't add enything
	if (strlen(Lcl_languages[Lcl_current_lang].lang_ext) <= 0) {
		return;
	}

	// get the length of the string so far
	path_len = strlen(current_path);
	if (path_len <= 0) {
		return;
	}
	
	// get the current last char
	last_char = current_path[path_len - 1];

	// if the last char is a slash, just copy in the disk extension
	if (last_char == DIR_SEPARATOR_CHAR) {
		strcat(current_path, Lcl_languages[Lcl_current_lang].lang_ext);
		strcat(current_path, DIR_SEPARATOR_STR);
	} 
	// otherwise add a slash, then copy in the disk extension
	else {
		strcat(current_path, DIR_SEPARATOR_STR);
		strcat(current_path, Lcl_languages[Lcl_current_lang].lang_ext);
	}
}

// maybe add localized directory to full path with file name when opening a localized file
int lcl_add_dir_to_path_with_filename(char *current_path, size_t path_max)
{
	// if the disk extension is 0 length, don't add anything
	if (strlen(Lcl_languages[Lcl_current_lang].lang_ext) <= 0) {
		return 1;
	}

	size_t str_size = path_max + 1;

	char *temp = new char[str_size];
	memset(temp, 0, str_size * sizeof(char));

	// find position of last slash and copy rest of filename (not counting slash) to temp
	// mark end of current path with '\0', so strcat will work
	char *last_slash = strrchr(current_path, DIR_SEPARATOR_CHAR);
	if (last_slash == NULL) {
		strncpy(temp, current_path, path_max);
		current_path[0] = '\0';
	} else {
		strncpy(temp, last_slash+1, path_max);
		last_slash[1] = '\0';
	}

	// add extension
	strcat_s(current_path, path_max, Lcl_languages[Lcl_current_lang].lang_ext);
	strcat_s(current_path, path_max, DIR_SEPARATOR_STR );

	// copy rest of filename from temp
	strcat_s(current_path, path_max, temp);

	delete [] temp;
	return 1;
}


// externalization of table/mission files ----------------------- 

void lcl_replace_stuff(char *text, size_t max_len)
{
	if (Fred_running)
		return;

	Assert(text);	// Goober5000

	// delegate to SCP_string for the replacements
	SCP_string temp_text = text;
	lcl_replace_stuff(temp_text);

	// fill up the original string
	size_t len = temp_text.copy(text, max_len);
	text[len] = 0;
}

// Goober5000 - replace stuff in the string, e.g. $callsign with player's callsign
// now will also replace $rank with rank, e.g. "Lieutenant"
// now will also replace $quote with double quotation marks
// now will also replace $semicolon with semicolon mark
// now will also replace $slash and $backslash
void lcl_replace_stuff(SCP_string &text)
{
	if (Fred_running)
		return;

	if (Player != NULL)
	{
		replace_all(text, "$callsign", Player->callsign);
		replace_all(text, "$rank", Ranks[Player->stats.rank].name);
	}
	replace_all(text, "$quote", "\"");
	replace_all(text, "$semicolon", ";");
	replace_all(text, "$slash", "/");
	replace_all(text, "$backslash", "\\");
}

void lcl_fred_replace_stuff(char *text, size_t max_len)
{
	if (!Fred_running)
		return;

	Assert(text);	// Goober5000

	// delegate to SCP_string for the replacements
	SCP_string temp_text = text;
	lcl_fred_replace_stuff(temp_text);

	// fill up the original string
	size_t len = temp_text.copy(text, max_len);
	text[len] = 0;
}

void lcl_fred_replace_stuff(SCP_string &text)
{
	if (!Fred_running)
		return;

	replace_all(text, "\"", "$quote");
	replace_all(text, ";", "$semicolon");
	replace_all(text, "/", "$slash");
	replace_all(text, "\\", "$backslash");
}

// get the localized version of the string. if none exists, return the original string
// valid input to this function includes :
// "this is some text"
// XSTR("wheeee", -1)
// XSTR("whee", 20)
// and these should cover all the externalized string cases
// fills in id if non-NULL. a value of -2 indicates it is not an external string
void lcl_ext_localize_sub(const char *in, char *out, size_t max_len, int *id)
{
	char text_str[PARSE_BUF_SIZE]="";
	int str_id;
	size_t str_len;

	Assert(in);
	Assert(out);

	// default (non-external string) value
	if (id != NULL) {
		*id = -2;
	}

	str_len = strlen(in);

	// if the string is < 9 chars, it can't be an XSTR("",) tag, so just copy it
	if (str_len < 9) {
		if (str_len > max_len)
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", in, str_len, max_len);

		strncpy(out, in, max_len);

		if (id != NULL)
			*id = -2;

		return;
	}

	// otherwise, check to see if it's an XSTR() tag
	if (strnicmp(in, "XSTR", 4)) {
		// NOT an XSTR() tag
		if (str_len > max_len)
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", in, str_len, max_len);

		strncpy(out, in, max_len);

		if (id != NULL)
			*id = -2;

		return;
	}

	// at this point we _know_ its an XSTR() tag, so split off the strings and id sections
	if (!lcl_ext_get_text(in, text_str)) {
		if (str_len > max_len)
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", in, str_len, max_len);

		strncpy(out, in, max_len);

		if (id != NULL)
			*id = -1;

		return;
	}
	if (!lcl_ext_get_id(in, &str_id)) {
		if (str_len > max_len)
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", in, str_len, max_len);

		strncpy(out, in, max_len);

		if (id != NULL)
			*id = -1;

		return;
	}
	
	// if the localization file is not open, or we're running in the default language, return the original string
	if ( !Xstr_inited || (str_id < 0) || (Lcl_current_lang == FS2_OPEN_DEFAULT_LANGUAGE) ) {
		if ( strlen(text_str) > max_len )
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", text_str, strlen(text_str), max_len);

		strncpy(out, text_str, max_len);

		if (id != NULL)
			*id = str_id;

		return;
	}

	// get the string if it exists
	if ((str_id < LCL_MAX_STRINGS) && (Lcl_ext_str[str_id] != NULL)) {
		// copy to the outgoing string
		if ( strlen(Lcl_ext_str[str_id]) > max_len )
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", Lcl_ext_str[str_id], strlen(Lcl_ext_str[str_id]), max_len);

		strncpy(out, Lcl_ext_str[str_id], max_len);
	}
	// otherwise use what we have - probably should Int3() or assert here
	else {
		if ( strlen(text_str) > max_len )
			error_display(0, "Token too long: [%s].  Length = %i.  Max is %i.\n", text_str, strlen(text_str), max_len);

		if (str_id >= LCL_MAX_STRINGS)
			error_display(0, "Invalid XSTR ID: [%d]. (Must be less than %d.)\n", str_id, LCL_MAX_STRINGS);

		strncpy(out, text_str, max_len);
	}

	// set the id #
	if (id != NULL) {
		*id = str_id;
	}
}

// ditto for SCP_string
void lcl_ext_localize_sub(const SCP_string &in, SCP_string &out, int *id)
{
	SCP_string text_str = "";
	int str_id;

	// default (non-external string) value
	if (id != NULL) {
		*id = -2;
	}	

	// if the string is < 9 chars, it can't be an XSTR("",) tag, so just copy it
	if (in.length() < 9) {
		out = in;

		if (id != NULL)
			*id = -2;

		return;
	}

	// otherwise, check to see if it's an XSTR() tag
	if (in.compare(0, 4, "XSTR")) {
		// NOT an XSTR() tag
		out = in;

		if (id != NULL)
			*id = -2;

		return;
	}

	// at this point we _know_ its an XSTR() tag, so split off the strings and id sections		
	if (!lcl_ext_get_text(in, text_str)) {
		out = in;

		if (id != NULL)
			*id = -1;

		return;
	}
	if (!lcl_ext_get_id(in, &str_id)) {
		out = in;

		if (id != NULL)
			*id = -1;

		return;
	}
	
	// if the localization file is not open, or we're running in the default language, return the original string
	if ( !Xstr_inited || (str_id < 0) || (Lcl_current_lang == FS2_OPEN_DEFAULT_LANGUAGE) ) {
		out = text_str;

		if (id != NULL)
			*id = str_id;

		return;
	}

	// get the string if it exists
	if ((str_id < LCL_MAX_STRINGS) && (Lcl_ext_str[str_id] != NULL)) {
		// copy to the outgoing string
		out = Lcl_ext_str[str_id];
	}
	// otherwise use what we have - probably should Int3() or assert here
	else {
		if (str_id >= LCL_MAX_STRINGS)
			error_display(0, "Invalid XSTR ID: [%d]. (Must be less than %d.)\n", str_id, LCL_MAX_STRINGS);

		out = text_str;
	}

	// set the id #
	if (id != NULL){
		*id = str_id;
	}
}

// Goober5000 - wrapper for lcl_ext_localize_sub; used because lcl_replace_stuff has to
// be called *after* the translation is done, and the original function returned in so
// many places that it would be messy to call lcl_replace_stuff everywhere
void lcl_ext_localize(const char *in, char *out, size_t max_len, int *id)
{
	// do XSTR translation
	lcl_ext_localize_sub(in, out, max_len, id);

	// do translation of $callsign, $rank, etc.
	lcl_replace_stuff(out, max_len);
}

// ditto for SCP_string
void lcl_ext_localize(const SCP_string &in, SCP_string &out, int *id)
{
	// do XSTR translation
	lcl_ext_localize_sub(in, out, id);

	// do translation of $callsign, $rank, etc.
	lcl_replace_stuff(out);
}

// translate the specified string based upon the current language
const char *XSTR(const char *str, int index)
{
	if(!Xstr_inited)
	{
		Int3();
		return str;
	}

	// perform a lookup
	if (index >= 0 && index < XSTR_SIZE)
	{
		// return translation of string
		if (Xstr_table[index].str)
			return Xstr_table[index].str;
	}

	// can't translate; return original english string
	return str;
}

// retrieve the offset for a localized string
int lcl_get_xstr_offset(int index, int res)
{
	if (res == GR_640) {
		return Xstr_table[index].offset_x;
	} else {
		return Xstr_table[index].offset_x_hi;
	}
}


// ------------------------------------------------------------------------------------------------------------
// LOCALIZE FORWARD DEFINITIONS
//

// given a valid XSTR() tag piece of text, extract the string portion, return it in out, nonzero on success
int lcl_ext_get_text(const char *xstr, char *out)
{
	int str_start, str_end;
	int str_len;
	const char *p, *p2;

	Assert(xstr != NULL);
	Assert(out != NULL);
	str_len = strlen(xstr);
	
	// this is some crazy wack-ass code.
	// look for the open quote
	str_start = str_end = 0;
	p = strstr(xstr, "\"");
	if(p == NULL){
		error_display(0, "Error parsing XSTR() tag %s\n", xstr);
		return 0;
	} else {
		str_start = p - xstr + 1;		
	}
	// make sure we're not about to walk past the end of the string
	if((p - xstr) >= str_len){
		error_display(0, "Error parsing XSTR() tag %s\n", xstr);
		return 0;
	}

	// look for the close quote
	p2 = strstr(p+1, "\"");
	if(p2 == NULL){
		error_display(0, "Error parsing XSTR() tag %s\n", xstr);
		return 0;
	} else {
		str_end = p2 - xstr;
	}

	// check bounds
	if (str_end - str_start > PARSE_BUF_SIZE - 1) {
		error_display(0, "String cannot fit within XSTR buffer!\n\n%s\n", xstr);
		return 0;
	}

	// now that we know the boundaries of the actual string in the XSTR() tag, copy it
	memcpy(out, xstr + str_start, str_end - str_start);	

	// success
	return 1;
}

// given a valid XSTR() tag piece of text, extract the string portion, return it in out, nonzero on success
int lcl_ext_get_text(const SCP_string &xstr, SCP_string &out)
{
	size_t open_quote_pos, close_quote_pos;

	// this is some crazy wack-ass code.
	// look for the open quote
	open_quote_pos = xstr.find('\"');
	if (open_quote_pos == SCP_string::npos) {
		error_display(0, "Error parsing XSTR() tag %s\n", xstr.c_str());
		return 0;
	}

	// look for the close quote
	close_quote_pos = xstr.find('\"', open_quote_pos+1);
	if (close_quote_pos == SCP_string::npos) {
		error_display(0, "Error parsing XSTR() tag %s\n", xstr.c_str());
		return 0;
	}

	// now that we know the boundaries of the actual string in the XSTR() tag, copy it
	out.assign(xstr, open_quote_pos + 1, close_quote_pos - open_quote_pos - 1);

	// success
	return 1;
}

// given a valid XSTR() tag piece of text, extract the id# portion, return the value in out, nonzero on success
int lcl_ext_get_id(const char *xstr, int *out)
{
	const char *p, *pnext;
	int str_len;

	Assert(xstr != NULL);
	Assert(out != NULL);
	
	str_len = strlen(xstr);

	// find the first quote
	p = strchr(xstr, '"');
	if(p == NULL){
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
		return 0;
	}
	// make sure we're not about to walk off the end of the string
	if((p - xstr) >= str_len){
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
		return 0;
	}
	p++;

	// continue searching until we find the close quote
	while(true){
		pnext = strchr(p, '"');
		if(pnext == NULL){
			error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
			return 0;
		}

		// if the previous char is a \, we know its not the "end-of-string" quote
		if(*(pnext - 1) != '\\'){
			p = pnext;
			break;
		}

		// continue
		p = pnext;
	}

	// search until we find a ,	
	pnext = strchr(p, ',');
	if(pnext == NULL){
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
		return 0;
	}
	// make sure we're not about to walk off the end of the string
	if((pnext - xstr) >= str_len){
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
		return 0;
	}
	
	// now get the id string
	p = pnext+1;
	while (is_gray_space(*p))
		p++;
	pnext = strchr(p+1, ')');
	if(pnext == NULL){
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr);
		return 0;
	}
	if(pnext - p >= PARSE_ID_BUF_SIZE){
		error_display(0, "XSTR() id# is too long in %s\n", xstr);
		return 0;
	}
	char buf[PARSE_ID_BUF_SIZE];
	strncpy(buf, p, pnext - p);
	buf[pnext - p] = 0;

	// get the value and we're done
	*out = atoi(buf);

	// success
	return 1;
}

// given a valid XSTR() tag piece of text, extract the id# portion, return the value in out, nonzero on success
int lcl_ext_get_id(const SCP_string &xstr, int *out)
{
	char id_buf[10];
	size_t p, pnext;

	// find the first quote
	p = xstr.find('\"');
	if (p == SCP_string::npos) {
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
		return 0;
	}
	p++;

	// continue searching until we find the close quote
	while(1) {
		pnext = xstr.find('\"', p);
		if (pnext == SCP_string::npos) {
			error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
			return 0;
		}

		// if the previous char is a \, we know its not the "end-of-string" quote
		if (xstr[pnext - 1] != '\\') {
			p = pnext;
			break;
		}

		// continue
		p = pnext;
	}

	// search until we find a ,	
	pnext = xstr.find(',', p);
	if (pnext == SCP_string::npos) {
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
		return 0;
	}
	pnext++;

	// find the close parenthesis
	p = pnext;
	pnext = xstr.find(')', p);
	if (pnext == SCP_string::npos) {
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
		return 0;
	}
	pnext--;

	// get only the number
	while (is_white_space(xstr[p]) && p <= pnext)
		p++;
	while (is_white_space(xstr[pnext]) && p <= pnext)
		pnext--;
	if (p > pnext) {
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
		return 0;
	}

	// now get the id string
	if ((pnext - p + 1) > 9) {
		error_display(0, "Error parsing id# in XSTR() tag %s\n", xstr.c_str());
		return 0;
	}
	memset(id_buf, 0, 10);
	xstr.copy(id_buf, pnext - p + 1, p);

	// get the value and we're done
	*out = atoi(id_buf);

	// success
	return 1;
}

// if the char is a valid char for a signed integer value
int lcl_is_valid_numeric_char(char c)
{
	return ( (c == '-') || (c == '0') || (c == '1') || (c == '2') || (c == '3') || (c == '4') ||
				(c == '5') || (c == '6') || (c == '7') || (c == '8') || (c == '9') ) ? 1 : 0;
}

void lcl_get_language_name(char *lang_name)
{
	Assert(Lcl_current_lang < (int)Lcl_languages.size());

	strcpy(lang_name, Lcl_languages[Lcl_current_lang].lang_name);
}

// ------------------------------------------------------------------
// lcl_translate_wep_name_gr()
//
// For displaying weapon names in german version
// since we can't actually just change them outright.
//
void lcl_translate_wep_name_gr(char *name)
{
	if (!strcmp(name, "Morning Star")) {	
		strcpy(name, "Morgenstern");
	} else if (!strcmp(name, "MorningStar")) {
		strcpy(name, "Morgenstern D");
	} else if (!strcmp(name, "UD-8 Kayser")) {
		strcpy(name, "Kayserstrahl");
	} else if (!strcmp(name, "UD-D Kayser")) {
		strcpy(name, "Kayserstrahl");
	}
}

// ------------------------------------------------------------------
// lcl_translate_brief_icon_name_gr()
//
// For displaying ship names in german version
// since we can't actually just change them outright.
//
void lcl_translate_brief_icon_name_gr(char *name)
{
	char *pos;
	char buf[128];

	if (!stricmp(name, "Subspace Portal")) {	
		strcpy(name, "Subraum Portal");

	} else if (!stricmp(name, "Alpha Wing")) {
		strcpy(name, "Alpha");

	} else if (!stricmp(name, "Beta Wing")) {
		strcpy(name, "Beta");

	} else if (!stricmp(name, "Zeta Wing")) {
		strcpy(name, "Zeta");

	} else if (!stricmp(name, "Capella Node")) {
		strcpy(name, "Capella");

	} else if (!stricmp(name, "Hostile")) {
		strcpy(name, "Gegner");

	} else if (!stricmp(name, "Hostile Craft")) {
		strcpy(name, "Gegner");

	} else if (!stricmp(name, "Rebel Wing")) {
		strcpy(name, "Rebellen");

	} else if (!stricmp(name, "Rebel Fleet")) {
		strcpy(name, "Rebellenflotte");

	} else if (!stricmp(name, "Sentry Gun")) {
		strcpy(name, "Gesch\x81tz");

	} else if (!stricmp(name, "Cargo")) {
		strcpy(name, "Fracht");

	} else if (!stricmp(name, "Knossos Device")) {
		strcpy(name, "Knossosger\x84t");
	
	} else if (!stricmp(name, "Support")) {
		strcpy(name, "Versorger");

	} else if (!stricmp(name, "Unknown")) {
		strcpy(name, "Unbekannt");

	} else if (!stricmp(name, "Instructor")) {
		strcpy(name, "Ausbilder");
	
	} else if (!stricmp(name, "Jump Node")) {
		strcpy(name, "Sprungknoten");

	} else if (!stricmp(name, "Escort")) {
		strcpy(name, "Geleitschutz");

	} else if (!stricmp(name, "Asteroid Field")) {
		strcpy(name, "Asteroidenfeld");

	} else if (!stricmp(name, "Enif Station")) {
		strcpy(name, "Station Enif");

	} else if (!stricmp(name, "Rally Point")) {
		strcpy(name, "Sammelpunkt");

	} else if ((pos = strstr(name, "Transport")) != NULL) {
		pos += 9;		// strlen of "transport"
		strcpy_s(buf, "Transportowiec");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Jump Node")) != NULL) {
		pos += 9;		// strlen of "jump node"
		strcpy_s(buf, "Sprungknoten");
		strcat_s(buf, pos);
		strcpy(name, buf);
	
	} else if (!stricmp(name, "Orion under repair")) {
		strcpy(name, "Orion wird repariert");

	// SOTY-specific ones below!
	
	} else if (!stricmp(name, "Wayfarer Station")) {
		strcpy(name, "Station Wayfarer");
	} else if (!stricmp(name, "Enemy")) {
		strcpy(name, "Gegner");
	} else if (!stricmp(name, "Supply Depot")) {
		strcpy(name, "Nachschubdepot");
	} else if (!stricmp(name, "Fighter Escort")) {
		strcpy(name, "Jagdschutz");
	} else if (!stricmp(name, "Shivans")) {
		strcpy(name, "Shivaner");
	} else if (!stricmp(name, "NTF Base of Operations")) {
		strcpy(name, "NTF-Operationsbasis");
	} else if (!stricmp(name, "NTF Bombers")) {
		strcpy(name, "NTF-Bomber");
	} else if (!stricmp(name, "NTF Fighters")) {
		strcpy(name, "NTF-J\x84ger");
	} else if (!stricmp(name, "Sentry")) {
		strcpy(name, "Sperrgesch\x81tz");
	} else if (!stricmp(name, "Cargo Containers")) {
		strcpy(name, "Frachtbeh\x84lter");
	} else if (!stricmp(name, "NTF Reinforcements")) {
		strcpy(name, "NTF-Verst\x84rkungen");
	} else if (!stricmp(name, "NTF Base")) {
		strcpy(name, "NTF-St\x81tzpunkt");
	} else if (!stricmp(name, "Refugee Convoy")) {
		strcpy(name, "Fl\x81""chtlingskonvoi");
	} else if (!stricmp(name, "Food Convoy")) {
		strcpy(name, "Nachschubkonvoi");
	} else if (!stricmp(name, "Governor's Shuttle")) {
		strcpy(name, "F\x84hre des Gouverneurs");
	} else if (!stricmp(name, "GTVA Patrol")) {
		strcpy(name, "GTVA-Patrouille");
	} else if (!stricmp(name, "Escort fighters")) {
		strcpy(name, "Geleitschutz");
	} else if (!stricmp(name, "Nagada Outpost")) {
		strcpy(name, "Nagada-Aussenposten");
	} else if (!stricmp(name, "Fighters")) {
		strcpy(name, "J\x84ger");
	} else if (!stricmp(name, "Bombers")) {
		strcpy(name, "Bomber");
	} else if (!stricmp(name, "Enemy Destroyers")) {
		strcpy(name, "Feindliche Zerst\x94rer");
	} else if (!stricmp(name, "Ross 128 System")) {
		strcpy(name, "System Ross 128");
	} else if (!stricmp(name, "Knossos Station")) {
		strcpy(name, "Knossos-Station");
	} else if (!stricmp(name, "Transporters")) {
		strcpy(name, "Transporter");
	} else if (!stricmp(name, "Pirates?")) {
		strcpy(name, "Piraten?");
	} else if (!stricmp(name, "Escorts")) {
		strcpy(name, "Geleitschutz");
	} else if (!stricmp(name, "Shivan Fighters")) {
		strcpy(name, "J\x84ger");
	} else if (!stricmp(name, "Shivan Territory")) {
		strcpy(name, "Shivaner");
	}
}

// ------------------------------------------------------------------
// lcl_translate_brief_icon_name_pl()
//
// For displaying ship names in polish version
// since we can't actually just change them outright.
//
void lcl_translate_brief_icon_name_pl(char *name)
{
char *pos;
char buf[128];
  	 
	if (!stricmp(name, "Subspace Portal")) {
		strcpy(name, "Portal podprz.");

	} else if (!stricmp(name, "Alpha Wing")) {
		strcpy(name, "Alfa");

	} else if (!stricmp(name, "Beta Wing")) {
		strcpy(name, "Beta");

	} else if (!stricmp(name, "Zeta Wing")) {
		strcpy(name, "Zeta");

	} else if (!stricmp(name, "Capella Node")) {
		strcpy(name, "Capella");

	} else if (!stricmp(name, "Hostile")) {
		strcpy(name, "Wr\xF3g");

	} else if (!stricmp(name, "Hostile Craft")) {
		strcpy(name, "Wr\xF3g");

	} else if (!stricmp(name, "Rebel Wing")) {
		strcpy(name, "Rebelianci");

	} else if (!stricmp(name, "Rebel Fleet")) {
		strcpy(name, "Flota Rebelii");

	} else if (!stricmp(name, "Sentry Gun")) {
		strcpy(name, "Dzia\xB3o str.");

	} else if (!stricmp(name, "Cargo")) {
		strcpy(name, "\xA3\x61\x64unek");

	} else if (!stricmp(name, "Knossos Device")) {
		strcpy(name, "Urz. Knossos");

	} else if (!stricmp(name, "Support")) {
		strcpy(name, "Wsparcie");

	} else if (!stricmp(name, "Unknown")) {
		strcpy(name, "Nieznany");

	} else if (!stricmp(name, "Instructor")) {
		strcpy(name, "Instruktor");

	} else if (!stricmp(name, "Jump Node")) {
		strcpy(name, "W\xEAze\xB3 skokowy");

	} else if (!stricmp(name, "Escort")) {
		strcpy(name, "Eskorta");

	} else if (!stricmp(name, "Asteroid Field")) {
		strcpy(name, "Pole asteroid");

	} else if (!stricmp(name, "Enif Station")) {
		strcpy(name, "Stacja Enif");

	} else if (!stricmp(name, "Rally Point")) {
		strcpy(name, "Pkt zborny");

	} else if ((pos = strstr(name, "Transport")) != NULL) {
		pos += 9;		// strlen of "transport"
		strcpy_s(buf, "Transporter");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Jump Node")) != NULL) {
		pos += 9;		// strlen of "jump node"
		strcpy_s(buf, "W\xEAze\xB3 skokowy");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if (!stricmp(name, "Orion under repair")) {
		strcpy(name, "Naprawiany Orion");
	}
}

// ------------------------------------------------------------------
// lcl_translate_ship_name_gr()
//
// For displaying ship names in german version in the briefing
// since we can't actually just change them outright.
//
void lcl_translate_ship_name_gr(char *name)
{
	if (!strcmp(name, "GTDR Amazon Advanced")) {	
		strcpy(name, "GTDR Amazon VII");
	} 
}

// ------------------------------------------------------------------
// lcl_translate_targetbox_name_gr()
//
// For displaying ship names in german version in the targetbox
// since we can't actually just change them outright.
//
void lcl_translate_targetbox_name_gr(char *name)
{
	char *pos;
	char buf[128];
	
	if ((pos = strstr(name, "Sentry")) != NULL) {
		pos += 6;		// strlen of "sentry"
		strcpy_s(buf, "Sperrgesch\x81tz");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Support")) != NULL) {
		pos += 7;		// strlen of "support"
		strcpy_s(buf, "Versorger");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Unknown")) != NULL) {
		pos += 7;		// strlen of "unknown"
		strcpy_s(buf, "Unbekannt");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Drone")) != NULL) {
		pos += 5;		// strlen of "drone"
		strcpy_s(buf, "Drohne");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Jump Node")) != NULL) {
		pos += 9;		// strlen of "jump node"
		strcpy_s(buf, "Sprungknoten");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if (!stricmp(name, "Instructor")) {
		strcpy(name, "Ausbilder");

	} else if (!stricmp(name, "NTF Vessel")) {
		strcpy(name, "NTF-Schiff");

	} else if (!stricmp(name, "Enif Station")) {
		strcpy(name, "Station Enif");
	}
}

// ------------------------------------------------------------------
// lcl_translate_targetbox_name_pl()
//
// For displaying ship names in polish version in the targetbox
// since we can't actually just change them outright.
//
void lcl_translate_targetbox_name_pl(char *name)
{
	char *pos;
	char buf[128];

	if ((pos = strstr(name, "Sentry")) != NULL) {
		pos += 6;		// strlen of "sentry"
		strcpy_s(buf, "Stra\xBFnik");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Support")) != NULL) {
		pos += 7;		// strlen of "support"
		strcpy_s(buf, "Wsparcie");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Unknown")) != NULL) {
		pos += 7;		// strlen of "unknown"
		strcpy_s(buf, "Nieznany");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Drone")) != NULL) {
		pos += 5;		// strlen of "drone"
		strcpy_s(buf, "Sonda");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if ((pos = strstr(name, "Jump Node")) != NULL) {
		pos += 9;		// strlen of "jump node"
		strcpy_s(buf, "W\xEAze\xB3 skokowy");
		strcat_s(buf, pos);
		strcpy(name, buf);

	} else if (!stricmp(name, "Instructor")) {
		strcpy(name, "Instruktor");

	} else if (!stricmp(name, "NTF Vessel")) {
		strcpy(name, "Okr\xEAt NTF");

	} else if (!stricmp(name, "Enif Station")) {
		strcpy(name, "Stacja Enif");
	}
}

// this is just a hack to display translated names without actually changing the names, 
// which would break stuff
// (this used to be in medals.cpp)
void lcl_translate_medal_name_gr(char *name)
{
	if (!strcmp(name, "Epsilon Pegasi Liberation")) {
		strcpy(name, "Epsilon Pegasi Befreiungsmedaille");

	} else if (!strcmp(name, "Imperial Order of Vasuda")) {
		strcpy(name, "Imperialer Orden von Vasuda ");

	} else if (!strcmp(name, "Distinguished Flying Cross")) {
		strcpy(name, "Fliegerkreuz Erster Klasse");

	} else if (!strcmp(name, "SOC Service Medallion")) {
		strcpy(name, "SEK-Dienstmedaille ");

	} else if (!strcmp(name, "Intelligence Cross")) {
		strcpy(name, "Geheimdienstkreuz am Bande");

	} else if (!strcmp(name, "Order of Galatea")) {
		strcpy(name, "Orden von Galatea ");

	} else if (!strcmp(name, "Meritorious Unit Commendation")) {
		strcpy(name, "Ehrenspange der Allianz");

	} else if (!strcmp(name, "Medal of Valor")) {
		strcpy(name, "Tapferkeitsmedaille ");

	} else if (!strcmp(name, "GTVA Legion of Honor")) {
		strcpy(name, "Orden der GTVA-Ehrenlegion");

	} else if (!strcmp(name, "Allied Defense Citation")) {
		strcpy(name, "Alliierte Abwehrspange ");

	} else if (!strcmp(name, "Nebula Campaign Victory Star")) {
		strcpy(name, "Nebel-Siegesstern");

	} else if (!strcmp(name, "NTF Campaign Victory Star")) {
		strcpy(name, "NTF-Siegesstern ");

	} else if (!strcmp(name, "Rank")) {
		strcpy(name, "Dienstgrad");

	} else if (!strcmp(name, "Wings")) {
		strcpy(name, "Fliegerspange");

	} else if (!strcmp(name, "Ace")) {
		strcpy(name, "Flieger-As");

	} else if (!strcmp(name, "Double Ace")) {
		strcpy(name, "Doppel-As ");

	} else if (!strcmp(name, "Triple Ace")) {
		strcpy(name, "Dreifach-As ");
		
	} else if (!strcmp(name, "SOC Unit Crest")) {
		strcpy(name, "SEK-Abzeichen ");
	}
}

// this is just a hack to display translated names without actually changing the names, 
// which would break stuff
// (this used to be in medals.cpp)
void lcl_translate_medal_name_pl(char *name)
{
	if (!strcmp(name, "Epsilon Pegasi Liberation")) {
		strcpy(name, "Order Wyzwolenia Epsilon Pegasi");

	} else if (!strcmp(name, "Imperial Order of Vasuda")) {
		strcpy(name, "Imperialny Order Vasudy");

	} else if (!strcmp(name, "Distinguished Flying Cross")) {
		strcpy(name, "Krzy\xBF Wybitnego Pilota");

	} else if (!strcmp(name, "SOC Service Medallion")) {
		strcpy(name, "Krzy\xBF S\xB3u\xBF\x62 Specjalnych");

	} else if (!strcmp(name, "Intelligence Cross")) {
		strcpy(name, "Krzy\xBF Wywiadu");

	} else if (!strcmp(name, "Order of Galatea")) {
		strcpy(name, "Order Galatei");

	} else if (!strcmp(name, "Meritorious Unit Commendation")) {
		strcpy(name, "Medal Pochwalny");

	} else if (!strcmp(name, "Medal of Valor")) {
		strcpy(name, "Medal za Odwag\xEA");

	} else if (!strcmp(name, "GTVA Legion of Honor")) {
		strcpy(name, "Legia Honorowa GTVA");

	} else if (!strcmp(name, "Allied Defense Citation")) {
		strcpy(name, "Order za Obron\xEA Sojuszu");

	} else if (!strcmp(name, "Nebula Campaign Victory Star")) {
		strcpy(name, "Gwiazda Wiktorii Kampanii w Mg\xB3\x61wicy");

	} else if (!strcmp(name, "NTF Campaign Victory Star")) {
		strcpy(name, "Gwiazda Wiktorii Kampanii NTF");

	} else if (!strcmp(name, "Rank")) {
		strcpy(name, "Ranga");

	} else if (!strcmp(name, "Wings")) {
		strcpy(name, "Skrzyd\xB3\x61");

	} else if (!strcmp(name, "Ace")) {
		strcpy(name, "As");	

	} else if (!strcmp(name, "Double Ace")) {
		strcpy(name, "Podw\xF3jny As");

	} else if (!strcmp(name, "Triple Ace")) {
		strcpy(name, "Potr\xF3jny As");
		
	} else if (!strcmp(name, "SOC Unit Crest")) {
		strcpy(name, "Tarcza S\xB3u\xBF\x62 Specjalnych");	
	}
}

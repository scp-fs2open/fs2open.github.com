/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#ifndef _PARSELO_H
#define _PARSELO_H

#include "cfile/cfile.h"
#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"
#include "def_files/def_files.h"
#include "utils/unicode.h"

#include <cinttypes>
#include <exception>
#include <limits.h>

// NOTE: although the main game doesn't need this anymore, FRED2 still does
#define	PARSE_TEXT_SIZE	1000000

extern char Current_filename[MAX_PATH_LEN];
extern char	*Parse_text;
extern char	*Parse_text_raw;
extern char	*Mp;
extern const char	*token_found;
extern int fred_parse_flag;
extern int Token_found_flag;


#define	COMMENT_CHAR	(char)';'
#define	EOLN			(char)0x0a
#define CARRIAGE_RETURN (char)0x0d

#define	F_NAME					1
#define	F_DATE					2
#define	F_NOTES					3
#define	F_FILESPEC				4
#define	F_MULTITEXTOLD			5	// needed for backwards compatability with old briefing format
#define	F_SEXP					6
#define	F_PATHNAME				7
#define	F_SHIPCHOICE			8
#define	F_MESSAGE				9	// this is now obsolete for mission messages - all messages in missions should now use $MessageNew and stuff strings as F_MULTITEXT
#define	F_MULTITEXT				10
#define F_RAW					11	// for any internal parsing use. Just strips whitespace and copies the text.
#define F_LNAME					12	//Filenames

#define PARSE_BUF_SIZE			4096

#define	SHIP_TYPE			0	// used to identify which kind of array to do a search for a name in
#define	SHIP_INFO_TYPE		1
#define	WEAPON_LIST_TYPE	2	//	to parse an int_list of weapons
#define	RAW_INTEGER_TYPE	3	//	to parse a list of integers
#define	WEAPON_POOL_TYPE	4

// Karajorma - Used by the stuff_ship_list and stuff_weapon_list SEXPs
#define NOT_SET_BY_SEXP_VARIABLE	-1

#define MISSION_LOADOUT_SHIP_LIST		5
#define MISSION_LOADOUT_WEAPON_LIST		6
#define CAMPAIGN_LOADOUT_SHIP_LIST		7
#define CAMPAIGN_LOADOUT_WEAPON_LIST	8

#define SEXP_SAVE_MODE				1
#define SEXP_ERROR_CHECK_MODE		2

// Goober5000 - this seems to be a pretty universal function
extern bool end_string_at_first_hash_symbol(char *src, bool ignore_doubled_hash = false);
extern bool end_string_at_first_hash_symbol(SCP_string &src, bool ignore_doubled_hash = false);
extern char *get_pointer_to_first_hash_symbol(char *src, bool ignore_doubled_hash = false);
extern const char *get_pointer_to_first_hash_symbol(const char *src, bool ignore_doubled_hash = false);
extern int get_index_of_first_hash_symbol(SCP_string &src, bool ignore_doubled_hash = false);

extern void consolidate_double_characters(char *str, char ch);

// white space
extern int is_white_space(char ch);
extern int is_white_space(unicode::codepoint_t cp);
extern void ignore_white_space(const char **pp = nullptr);
extern void drop_trailing_white_space(char *str);
extern void drop_leading_white_space(char *str);
extern char *drop_white_space(char *str);

// SCP_string white space
extern void drop_trailing_white_space(SCP_string &str);
extern void drop_leading_white_space(SCP_string &str);
extern void drop_white_space(SCP_string &str);

// gray space
extern int is_gray_space(char ch);
extern bool is_gray_space(unicode::codepoint_t cp);
extern void ignore_gray_space(const char **pp = nullptr);

// error
extern int get_line_num();
extern char *next_tokens(bool terminate_before_parenthesis_or_comma = false);
extern void diag_printf(SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(1, 2);
extern void error_display(int error_level, SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(2, 3);

// skip
extern int skip_to_string(const char *pstr, const char *end = NULL);
extern int skip_to_start_of_string(const char *pstr, const char *end = NULL);
extern int skip_to_start_of_string_either(const char *pstr1, const char *pstr2, const char *end = NULL);
extern void advance_to_eoln(const char *terminators);
extern bool skip_eoln();
extern void skip_token();

// optional
extern int optional_string(const char *pstr);
extern int optional_string_either(const char *str1, const char *str2, bool advance = true);
extern int optional_string_one_of(int arg_count, ...);

// required
extern int required_string(const char *pstr);
extern int required_string_either(const char *str1, const char *str2);
extern int required_string_one_of(int arg_count, ...);

// stuff
extern void copy_to_eoln(char *outstr, const char *more_terminators, const char *instr, int max);
extern void copy_text_until(char *outstr, const char *instr, const char *endstr, int max_chars);
extern void stuff_string_white(char *outstr, int len = 0);
extern void stuff_string_until(char *outstr, const char *endstr, int len = 0);
extern void stuff_string(char *outstr, int type, int len, const char *terminators = NULL);
extern void stuff_string_line(char *outstr, int len);

// SCP_string stuff
extern void copy_to_eoln(SCP_string &outstr, const char *more_terminators, const char *instr);
extern void copy_text_until(SCP_string &outstr, const char *instr, const char *endstr);
extern void stuff_string_white(SCP_string &outstr);
extern void stuff_string_until(SCP_string &outstr, const char *endstr);
extern void stuff_string(SCP_string &outstr, int type, const char *terminators = NULL);
extern void stuff_string_line(SCP_string &outstr);

//alloc
extern char* alloc_block(const char* startstr, const char* endstr, int extra_chars = 0);

// Exactly the same as stuff string only Malloc's the buffer.
//	Supports various FreeSpace primitive types.  If 'len' is supplied, it will override
// the default string length if using the F_NAME case.
extern char *stuff_and_malloc_string(int type, const char *terminators = nullptr);
extern void stuff_malloc_string(char **dest, int type, const char *terminators = nullptr);
extern int stuff_float(float *f, bool optional = false);
extern int stuff_int(int *i, bool optional = false);
extern int stuff_long(long *l, bool optional = false);
extern void stuff_ubyte(ubyte *i);
extern int stuff_int_optional(int *i);
extern int stuff_float_optional(float *f);
extern void stuff_string_list(SCP_vector<SCP_string>& slp);
extern size_t stuff_string_list(char slp[][NAME_LENGTH], size_t max_strings);
extern void parse_string_flag_list(int *dest, flag_def_list defs[], size_t defs_size);


// A templated version of parse_string_flag_list, to go along with the templated flag_def_list_new.
// If the "is_special" flag is set, or a string was not found in the def list, it will be added to the unparsed_or_special_strings Vector
// so that you can process it properly later. If you just want to handle special flags (that is, flags with arguments or special code on parse),
// consider using parse_string_flag_list_special and a special_flag_def_list_new instead.
template<class Flagdef, class Flagset>
void parse_string_flag_list(Flagset& dest, const Flagdef defs[], size_t n_defs, SCP_vector<SCP_string>* unparsed_or_special_strings)
{
	SCP_vector<SCP_string> slp;
    stuff_string_list(slp);

	for (auto &item : slp)
    {
        bool string_parsed = false;
        for (size_t j = 0; j < n_defs; j++)
        {
            if (!stricmp(item.c_str(), defs[j].name)) {
				if (defs[j].in_use) {
					Assertion(defs[j].def != decltype(Flagdef::def)::NUM_VALUES, "Error in definition for flag_def_list, flag '%s' has been given an invalid value but is still marked as in use.\n", defs[j].name);
					dest.set(defs[j].def);
				}

                if (!defs[j].is_special)
                    string_parsed = true;
            }
        }
        if (!string_parsed && unparsed_or_special_strings != NULL) {
            unparsed_or_special_strings->push_back(item);
        }
    }
}

// Like parse_string_flag_list, but capable of automatically handling special flags registered in the special_flag_def_list_new
template<class T, class Flagset, size_t n, typename... additional_args, typename... additional_args_fwd>
void parse_string_flag_list_special(Flagset& dest, const special_flag_def_list_new<T, additional_args...>(&defs)[n], SCP_vector<SCP_string>* unparsed_strings, additional_args_fwd&&... args) {
	SCP_vector<SCP_string> unparsed;
	parse_string_flag_list<special_flag_def_list_new<T, additional_args...>, Flagset>(dest, defs, n, &unparsed);
	for (const auto& special : unparsed) {
		const auto* const it = std::find_if(&defs[0], &defs[n], [&special](const special_flag_def_list_new<T, additional_args...>& flag) {
			if (!flag.is_special || !flag.parse_special)
				return false;
			return strnicmp(special.c_str(), flag.name, strlen(flag.name)) == 0;
			});

		if (it != &defs[n]) {
			const size_t flag_length = strlen(it->name);
			const size_t skip_length = flag_length + strspn(&special.c_str()[flag_length], NOX(" \t"));
			it->parse_special(special.substr(skip_length), std::forward<additional_args_fwd>(args)...);
		}
		else if (unparsed_strings != nullptr) {
			unparsed_strings->emplace_back(special);
		}
	}
}

template<class T>
void stuff_flagset(T *dest) {
    long l = 0;
    stuff_long(&l);

	if (l < 0) {
		error_display(0, "Expected flagset value but got negative value %lu!\n", l);
		l = 0;
	}
    dest->from_u64((std::uint64_t) l);

    diag_printf("Stuffed flagset: %" PRIu64 "\n", dest->to_u64());
}

extern size_t stuff_int_list(int *ilp, size_t max_ints, int lookup_type = RAW_INTEGER_TYPE);
extern size_t stuff_float_list(float* flp, size_t max_floats);
extern void stuff_float_list(SCP_vector<float>& flp);
extern size_t stuff_vec3d_list(vec3d *vlp, size_t max_vecs);
extern void stuff_vec3d_list(SCP_vector<vec3d> &vec_list);
extern size_t stuff_bool_list(bool *blp, size_t max_bools);
extern void stuff_vec3d(vec3d *vp);
extern void stuff_matrix(matrix *mp);
extern void stuff_angles_deg_phb(angles* vp);
extern void find_and_stuff(const char *id, int *addr, int f_type, const char *strlist[], size_t max, const char *description);
extern void find_and_stuff_optional(const char *id, int *addr, int f_type, const char * const *strlist, size_t max, const char *description);
extern int match_and_stuff(int f_type, const char * const *strlist, int max, const char *description);
extern void find_and_stuff_or_add(const char *id, int *addr, int f_type, char *strlist[], int *total,
	int max, const char *description);
extern int get_string(char *str, int max = -1);
extern void get_string(SCP_string &str);
extern void stuff_parenthesized_vec3d(vec3d *vp);
extern void stuff_boolean(int *i, bool a_to_eol=true);
extern void stuff_boolean(bool *b, bool a_to_eol=true);
extern void stuff_boolean_flag(int *i, int flag, bool a_to_eol=true);

template <class T>
int string_lookup(const char* str1, T strlist, size_t max, const char* description = nullptr, bool say_errors = false)
{
	for (size_t i=0; i<max; i++)
	{
		Assert(strlen(strlist[i]) != 0); //-V805

		if (!stricmp(str1, strlist[i]))
			return (int)i;
	}

	if (say_errors)
		error_display(0, "Unable to find [%s] in %s list.\n", str1, description);

	return -1;
}

template<class Flags, class Flagset>
void stuff_boolean_flag(Flagset& destination, Flags flag, bool a_to_eol = true)
{
    bool temp;
    stuff_boolean(&temp, a_to_eol);
    destination.set(flag, temp);
}

extern int check_for_string(const char *pstr);
extern int check_for_string_raw(const char *pstr);
extern int check_for_eof();
extern int check_for_eof_raw();
extern int check_for_eoln();

// from aicode.cpp
extern void parse_float_list(float *plist, size_t size);
extern void parse_int_list(int *ilist, size_t size);

extern void parse_string_map(SCP_map<SCP_string, SCP_string>& mapOut, const char* end_marker, const char* entry_prefix);

// general
extern void reset_parse(char *text = NULL);
extern void display_parse_diagnostics();
extern void pause_parse();
extern void unpause_parse();
// stop parsing, basically just frees up the memory from Parse_text and Parse_text_raw
extern void stop_parse();

// utility
extern void compact_multitext_string(char *str);
extern void compact_multitext_string(SCP_string &str);
extern void read_file_text(const char *filename, int mode = CF_TYPE_ANY, char *processed_text = NULL, char *raw_text = NULL);
extern void read_file_text_from_default(const default_file& file, char *processed_text = NULL, char *raw_text = NULL);
extern void read_raw_file_text(const char *filename, int mode = CF_TYPE_ANY, char *raw_text = NULL);
extern void process_raw_file_text(char *processed_text = NULL, char *raw_text = NULL);
extern void coerce_to_utf8(SCP_string &buffer, const char *src);
extern void debug_show_mission_text();
extern void convert_sexp_to_string(SCP_string &dest, int cur_node, int mode);
extern size_t maybe_convert_foreign_characters(const char *in, char *out, bool add_null = true);
extern void maybe_convert_foreign_characters(SCP_string &text);
extern size_t get_converted_string_length(const char *text);
extern size_t get_converted_string_length(const SCP_string &text);
char *split_str_once(char *src, int max_pixel_w);
int split_str(const char* src,
			  int max_pixel_w,
			  int* n_chars,
			  const char** p_str,
			  int max_lines,
			  int max_line_length = INT_MAX,
			  unicode::codepoint_t ignore_char = (unicode::codepoint_t) -1,
			  bool strip_leading_whitespace = true);
int split_str(const char* src,
			  int max_pixel_w,
			  SCP_vector<int>& n_chars,
			  SCP_vector<const char*>& p_str,
			  int max_line_length = INT_MAX,
			  unicode::codepoint_t ignore_char = (unicode::codepoint_t) -1,
			  bool strip_leading_whitespace = true);

// fred
extern int required_string_fred(const char *pstr, const char *end = NULL);
extern int required_string_either_fred(const char *str1, const char *str2);
extern int optional_string_fred(const char *pstr, const char *end = NULL, const char *end2 = NULL);

// Goober5000
extern ptrdiff_t replace_one(char *str, const char *oldstr, const char *newstr, size_t max_len, ptrdiff_t range = 0);
extern ptrdiff_t replace_one(SCP_string& context, const SCP_string& from, const SCP_string& to);
extern ptrdiff_t replace_one(SCP_string& context, const char* from, const char* to);

// Goober5000
extern int replace_all(char *str, const char *oldstr, const char *newstr, size_t max_len, ptrdiff_t range = 0);
extern int replace_all(SCP_string& context, const SCP_string& from, const SCP_string& to);
extern int replace_all(SCP_string& context, const char* from, const char* to);

// Goober5000 (why is this not in the C library?)
extern const char *stristr(const char *str, const char *substr);
extern char *stristr(char *str, const char *substr);

// Goober5000 (ditto)
extern bool can_construe_as_integer(const char *text);

// Goober5000 (ditto for C++)
extern void vsprintf(SCP_string &dest, const char *format, va_list ap);
extern void sprintf(SCP_string &dest, SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(2, 3);

// Goober5000
extern int subsystem_stricmp(const char *str1, const char *str2);

//WMC - compares two strings, ignoring the last extension
extern int strextcmp(const char *s1, const char *s2);

// Goober5000 - truncates a file extension
extern bool drop_extension(char *str);
extern bool drop_extension(SCP_string &str);

//WMC - backspaces the first character of given char pointer
extern void backspace(char *src);

// Goober5000 - prints a properly comma-separated integer to a string
extern void format_integer_with_commas(char *buf, int integer, bool use_comma_with_four_digits);

// Goober5000
extern int scan_fso_version_string(const char *text, int *major, int *minor, int *build, int *revis);

// Goober5000
extern void truncate_message_lines(SCP_string &text, int num_allowed_lines);

inline void parse_advance(int s){Mp+=s;}

// parse a modular table, returns the number of files matching the "name_check" filter or 0 if it did nothing
extern int parse_modular_table(const char *name_check, void (*parse_callback)(const char *filename), int path_type = CF_TYPE_TABLES, int sort_type = CF_SORT_REVERSE);
// to know that we are parsing a modular table
extern bool Parsing_modular_table;

struct loadout_row
{
	int index = -1;
	int index_sexp_var = NOT_SET_BY_SEXP_VARIABLE;
	int count = -1;
	int count_sexp_var = NOT_SET_BY_SEXP_VARIABLE;
};

//Karajorma/Goober5000 - Parses mission and campaign ship loadouts.
void stuff_loadout_list(SCP_vector<loadout_row> &list, int lookup_type);
int get_string_or_variable (char *str);
int get_string_or_variable (SCP_string &str);
#define PARSING_FOUND_STRING		0
#define PARSING_FOUND_VARIABLE		1

namespace parse
{
	class ParseException : public std::runtime_error
	{
	public:
		explicit ParseException(const std::string& msg) : std::runtime_error(msg) {}
		~ParseException() noexcept override = default;
	};

	/**
	* @brief Parsing checkpoint
	*
	* @details Keeps track of the information it needs to be able to resume parsing a file at this location.
	*/
	class Bookmark
	{
	public:
		SCP_string filename;
		char* Mp;
		int Warning_count;
		int Error_count;
	};
}

#endif

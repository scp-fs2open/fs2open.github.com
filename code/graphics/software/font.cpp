/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>
#include <climits>

#include "graphics/software/font_internal.h"
#include "graphics/software/FontManager.h"
#include "graphics/software/FSFont.h"
#include "graphics/software/VFNTFont.h"
#include "graphics/software/NVGFont.h"

#include "graphics/2d.h"

#include "mod_table/mod_table.h"

#include "def_files/def_files.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "localization/localize.h"
#include "parse/parselo.h"
#include "tracing/Monitor.h"

namespace
{
	namespace fo = font;
	using namespace font;

	bool font_initialized = false;

	constexpr ubyte DEFAULT_SPECIAL_CHAR_INDEX = 0;

	// max allowed special char index; i.e. 7 special chars in retail fonts 1 & 3
	const int MAX_SPECIAL_CHAR_IDX = UCHAR_MAX - 6;


	bool parse_type(FontType &type, SCP_string &fileName)
	{
		int num = optional_string_either("$TrueType:", "$Font:");
		if (num == 0)
		{
			type = NVG_FONT;
		}
		else if (num == 1)
		{
			type = VFNT_FONT;
		}
		else
		{
			type = UNKNOWN_FONT;
		}


		if (type != UNKNOWN_FONT)
		{
			stuff_string(fileName, F_NAME);
			return true;
		}
		else
		{
			return false;
		}
	}

	void parse_nvg_font(const SCP_string& fontFilename)
	{
		float size = 8.0f;
		SCP_string fontStr;
		bool hasName = false;

		if (optional_string("+Name:"))
		{
			stuff_string(fontStr, F_NAME);

			hasName = true;
		}

		if (optional_string("+Size:"))
		{
			stuff_float(&size);

			if (size <= 0.0f)
			{
				error_display(0, "+Size has to be bigger than 0 for font \"%s\".", fontFilename.c_str());
				size = 8;
			}
		}

		// Build name from existing values if no name is specified
		if (!hasName)
		{
			SCP_stringstream ss;

			ss << fontFilename << "-";

			ss << size;

			fontStr = ss.str();
		}

		if (FontManager::getFont(fontStr) != NULL)
		{
			if (hasName)
			{
				error_display(0, "Font with name \"%s\" is already present! Font names have to be unique!", fontStr.c_str());
				return;
			}
			else
			{
				error_display(0, "Found font with same default name (\"%s\"). This is most likely a duplicate.", fontStr.c_str());
				return;
			}
		}

		NVGFont *nvgFont = FontManager::loadNVGFont(fontFilename, size);

		if (nvgFont == NULL)
		{
			error_display(0, "Couldn't load font \"%s\".", fontFilename.c_str());
			return;
		}

		if (optional_string("+Top offset:"))
		{
			float temp;

			stuff_float(&temp);

			nvgFont->setTopOffset(temp);
		}

		if (optional_string("+Bottom offset:"))
		{
			float temp;

			stuff_float(&temp);

			nvgFont->setBottomOffset(temp);
		}
		
		if (optional_string("+Tab width:"))
		{
			float temp;
			stuff_float(&temp);

			if (temp < 0.0f)
			{
				error_display(0, "Invalid tab spacing %f. Has to be greater or equal to zero.", temp);
			}
			else
			{
				nvgFont->setTabWidth(temp);
			}
		}

		if (optional_string("+Letter spacing:"))
		{
			float temp;
			stuff_float(&temp);

			if (temp < 0.0f)
			{
				error_display(0, "Invalid letter spacing %f! Has to be greater or equal to zero.", temp);
			}
			else
			{
				nvgFont->setLetterSpacing(temp);
			}
		}

		if (!Unicode_text_mode) {
			int special_char_index = DEFAULT_SPECIAL_CHAR_INDEX;
			// Special characters only exist in non-Unicode mode
			if (optional_string("+Special Character Font:"))
			{
				SCP_string fontName;
				stuff_string(fontName, F_NAME);

				fo::font* fontData = FontManager::loadFontOld(fontName.c_str());

				if (fontData == NULL)
				{
					error_display(0, "Failed to load font \"%s\" for special characters of font \"%s\"!", fontName.c_str(), fontFilename.c_str());
				}
				else
				{
					nvgFont->setSpecialCharacterFont(fontData);
				}

				if (optional_string("+Special Character Index:")) {
					stuff_int(&special_char_index);
					if (special_char_index < 0 || special_char_index >= MAX_SPECIAL_CHAR_IDX) {
						Warning(LOCATION, "Special character index (%d) for font (%s) is invalid, must be 0 - %u. Defaulting to 0.",
							special_char_index, fontName.c_str(), MAX_SPECIAL_CHAR_IDX);
						special_char_index = DEFAULT_SPECIAL_CHAR_INDEX;
					}
				}
				else {
					auto old_entry = FontManager::getFontByFilename(fontName);
					
					if (old_entry != nullptr) {
						int old_index = FontManager::getFontIndex(old_entry);
						special_char_index = Lcl_languages[0].special_char_indexes[old_index];
					} else {

						Warning(LOCATION,
							"A \"+Special Character Index:\" was not specified after the \"+Special Character Font:\" entry for the %s font. And a previous entry with a \"+Special Character Index:\" was not found."
							"\n\n This entry must be specified directly or indirectly to avoid this warning.  Defaulting to 0.",
							fontStr.c_str());
					}
				}

			}
			else
			{
				fo::font* fontData = FontManager::loadFontOld("font01.vf");

				if (fontData == nullptr) {
					error_display(0,
								  "Failed to load default font \"%s\" for special characters of font \"%s\"! "
									  "This font is required for rendering special characters, if another font is not defined, and will cause an error later.",
								  "font01.vf",
								  fontFilename.c_str());
				} else {
					nvgFont->setSpecialCharacterFont(fontData);
				}
			}

			auto font_id = FontManager::getFontIndex(nvgFont);

			// add the index specified to all languages
			for (auto & Lcl_language : Lcl_languages) {

				// if there wasn't already something specified for this font.
				if (font_id >= (int)Lcl_language.special_char_indexes.size()) {
					
					// if there were absolutely no special characters defined, put the default value as the language default.
					if (Lcl_language.special_char_indexes.empty() == 0) {
						Lcl_language.special_char_indexes.push_back(DEFAULT_SPECIAL_CHAR_INDEX);
					}

					// if somehow a lot of indexes were missing, add the language default to fill in the gaps until ...
					while (font_id > (int)Lcl_language.special_char_indexes.size()) {
							Lcl_language.special_char_indexes.push_back(Lcl_language.special_char_indexes[0]);
					}

					// we're at just the right place in the vector to add the index we just read from the table.
					Lcl_language.special_char_indexes.push_back((ubyte)special_char_index);

				} // replace if there was something already specified, except if nothing was specified in the table
				else if (special_char_index != DEFAULT_SPECIAL_CHAR_INDEX) {
					Lcl_language.special_char_indexes[font_id] = (ubyte)special_char_index;
				}
			}
		}

		nvgFont->setName(fontStr);
		nvgFont->setFilename(fontFilename);

		// Make sure that the height is not invalid
		nvgFont->computeFontMetrics();
	}

	void parse_vfnt_font(const SCP_string& fontFilename)
	{
		VFNTFont *font = FontManager::loadVFNTFont(fontFilename);

		if (font == NULL)
		{
			error_display(0, "Couldn't load font\"%s\".", fontFilename.c_str());
			return;
		}

		SCP_string fontName;

		if (optional_string("+Name:"))
		{
			stuff_string(fontName, F_NAME);
		}
		else
		{
			fontName.assign(fontFilename);
		}

		font->setName(fontName);
		font->setFilename(fontFilename);

		auto font_id = FontManager::getFontIndex(font);

		int user_defined_default_special_char_index = (int)DEFAULT_SPECIAL_CHAR_INDEX;
		// 'default' special char index for all languages using this font
		if (optional_string("+Default Special Character Index:")) {
			stuff_int(&user_defined_default_special_char_index);

			if (user_defined_default_special_char_index < 0 || user_defined_default_special_char_index >= MAX_SPECIAL_CHAR_IDX) {
				Warning(LOCATION, "Default special character index (%d) for font (%s), must be 0 - %u.  Defaulting to 0.", user_defined_default_special_char_index,
					  fontName.c_str(), MAX_SPECIAL_CHAR_IDX);
			}

		}

		// add the index specified to all the languages 
		for (auto & Lcl_language : Lcl_languages) {

			// if there wasn't already something specified for this font.
			if (font_id >= (int)Lcl_language.special_char_indexes.size()) {

				// if the default for the language was not already specified, set that default to the general default
				if (Lcl_language.special_char_indexes.empty()) {
					Lcl_language.special_char_indexes.push_back(DEFAULT_SPECIAL_CHAR_INDEX);
				}

				// if a lot of indexes were missing, add some defaults to fill in the gaps until ...
				while (font_id > (int)Lcl_language.special_char_indexes.size()) {
					// take into account retail special character settings for font index 2 (size is 1 indexed)
					if (Lcl_language.special_char_indexes.size() == FONT3 - 1) {
						Lcl_language.special_char_indexes.push_back(176);
					}
					else {
						Lcl_language.special_char_indexes.push_back(Lcl_language.special_char_indexes[0]);
					}
				}
				// we're at just the right place in the vector to add the index we just read from the table.
				Lcl_language.special_char_indexes.push_back((ubyte)DEFAULT_SPECIAL_CHAR_INDEX);
			} // if there was something already specified that's being replaced.
			else if (user_defined_default_special_char_index != DEFAULT_SPECIAL_CHAR_INDEX) {
				Lcl_language.special_char_indexes[font_id] = (ubyte)user_defined_default_special_char_index;
			}
		}

		while (optional_string("+Language:")) {
			char lang_name[LCL_LANG_NAME_LEN + 1];
			int special_char_index, lang_idx = -1;

			stuff_string(lang_name, F_NAME, LCL_LANG_NAME_LEN + 1);

			// find language and set the index, or if not found move to the next one
			for (auto i = 0; i < (int)Lcl_languages.size(); ++i) {
				if (!strcmp(Lcl_languages[i].lang_name, lang_name)) {
					lang_idx = i;
					break;
				}
			}

			if (lang_idx == -1) {
				Warning(LOCATION, "Ignoring invalid language (%s) specified by font (%s); not built-in or in strings.tbl",
						lang_name, fontName.c_str());
				skip_to_start_of_string_either("+Language:", "$Font:", "#End");
				continue;
			}

			if (optional_string("+Special Character Index:")) {
				stuff_int(&special_char_index);

				if (special_char_index < 0 || special_char_index >= MAX_SPECIAL_CHAR_IDX) {
					Error(LOCATION, "Special character index (%d) for font (%s), language (%s) is invalid, must be 0 - %u",
						  special_char_index, fontName.c_str(), lang_name, MAX_SPECIAL_CHAR_IDX);
				}

				Lcl_languages[lang_idx].special_char_indexes[font_id] = (ubyte)special_char_index;
			}
		}

		if (optional_string("+Top offset:"))
		{
			float temp;

			stuff_float(&temp);

			font->setTopOffset(temp);
		}

		if (optional_string("+Bottom offset:"))
		{
			float temp;

			stuff_float(&temp);

			font->setBottomOffset(temp);
		}
		// Make sure that the height is not invalid
		font->computeFontMetrics();
	}

	void font_parse_setup(const char *fileName)
	{
		bool noTable = false;
		if (!strcmp(fileName, "fonts.tbl"))
		{
			if (!cf_exists_full(fileName, CF_TYPE_TABLES))
			{
				noTable = true;
			}
		}

		if (noTable)
		{
			read_file_text_from_default(defaults_get_file("fonts.tbl"));
		}
		else
		{
			read_file_text(fileName, CF_TYPE_TABLES);
		}

		reset_parse();

		// start parsing
		required_string("#Fonts");
	}

	void parse_font_tbl(const char *fileName)
	{
		try
		{
			font_parse_setup(fileName);

			FontType type;
			SCP_string fontName;

			while (parse_type(type, fontName))
			{
				switch (type)
				{
				case VFNT_FONT:
					if (Unicode_text_mode) {
						error_display(1, "Bitmap fonts are not supported in Unicode text mode!");
					}
					parse_vfnt_font(fontName);
					break;
				case NVG_FONT:
					parse_nvg_font(fontName);
					break;
				default:
					error_display(0, "Unknown font type %d! Get a coder!", (int)type);
					break;
				}
			}

			// done parsing
			required_string("#End");
		}
		catch (const parse::ParseException& e)
		{
			mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", fileName, e.what()));
		}
	}

	void parse_fonts_tbl()
	{
		//Parse main TBL first
		parse_font_tbl("fonts.tbl");

		//Then other ones
		parse_modular_table("*-fnt.tbm", parse_font_tbl);

		// double check
		if (FontManager::numberOfFonts() < 3) {
			Error(LOCATION, "At least three fonts have to be loaded but only %d valid entries were found!", FontManager::numberOfFonts());
		}
	}
}

namespace font
{
	void init()
	{
		if (font_initialized) {
			// Already initialized
			return;
		}

		FontManager::init();

		parse_fonts_tbl();

		set_font(0);

		font_initialized = true;
	}

	void close()
	{
		if (!font_initialized) {
			return;
		}

		FontManager::close();

		font_initialized = false;
	}

	int force_fit_string(char *str, int max_str, int max_width)
	{
		int w;

		gr_get_string_size(&w, NULL, str);
		if (w > max_width) {
			if ((int)strlen(str) > max_str - 3) {
				Assert(max_str >= 3);
				str[max_str - 3] = 0;
			}

			strcpy(str + strlen(str) - 1, "...");
			gr_get_string_size(&w, NULL, str);
			while (w > max_width) {
				Assert(strlen(str) >= 4);  // if this is hit, a bad max_width was passed in and the calling function needs fixing.
				strcpy(str + strlen(str) - 4, "...");
				gr_get_string_size(&w, NULL, str);
			}
		}

		return w;
	}

	void stuff_first(SCP_string &firstFont)
	{
		try
		{
			font_parse_setup("fonts.tbl");

			FontType type;
			parse_type(type, firstFont);
		}
		catch (const parse::ParseException& e)
		{
			Error(LOCATION, "Failed to setup font parsing. This may be caused by an empty fonts.tbl file.\nError message: %s", e.what());
			firstFont = "";
		}
	}

	int parse_font()
	{
		int font_idx;

		SCP_string input;
		stuff_string(input, F_NAME);
		SCP_stringstream ss(input);

		int fontNum;
		ss >> fontNum;

		if (ss.fail())
		{
			fontNum = FontManager::getFontIndex(input);

			if (fontNum < 0)
			{
				error_display(0, "Invalid font name \"%s\"!", input.c_str());
				font_idx = -1;
			}
			else
			{
				font_idx = fontNum;
			}
		}
		else
		{
			if (fontNum < 0 || fontNum >= FontManager::numberOfFonts())
			{
				error_display(0, "Invalid font number %d! must be greater or equal to zero and smaller than %d.", fontNum, FontManager::numberOfFonts());
				font_idx = -1;
			}
			else
			{
				font_idx = fontNum;
			}
		}

		return font_idx;
	}

	FSFont *get_current_font()
	{
		return FontManager::getCurrentFont();
	}

	FSFont *get_font(const SCP_string& name)
	{
		return FontManager::getFont(name);
	}

	FSFont *get_font_by_filename(const SCP_string& name)
	{
		return FontManager::getFontByFilename(name);
	}

	

	/**
	* @brief	Gets the width of an character.
	*
	* Returns the width of the specified charachter also taking account of kerning.
	*
	* @param fnt				The font data
	* @param c1				The character that should be checked.
	* @param c2				The character which follows this character. Used to compute the kerning
	* @param [out]	width   	If non-null, the width.
	* @param [out]	spaceing	If non-null, the spaceing.
	*
	* @return	The character width.
	*/
	int get_char_width_old(fo::font* fnt, ubyte c1, ubyte c2, int *width, int* spacing)
	{
		int i, letter;

		letter = c1 - fnt->first_ascii;

		//not in font, draw as space
		if (letter < 0 || letter >= fnt->num_chars)
		{
			*width = 0;
			*spacing = fnt->w;
			return -1;
		}

		*width = fnt->char_data[letter].byte_width;
		*spacing = fnt->char_data[letter].spacing;

		i = fnt->char_data[letter].kerning_entry;
		if (i > -1)
		{
			if (!(c2 == 0 || c2 == '\n'))
			{
				int letter2;

				letter2 = c2 - fnt->first_ascii;

				if ((letter2 >= 0) && (letter2 < fnt->num_chars) && (i < fnt->num_kern_pairs))
				{
					font_kernpair *k = &fnt->kern_data[i];
					while ((i < fnt->num_kern_pairs) && (k->c1 == (char)letter) && (k->c2 < (char)letter2))
					{
						i++;
						k++;
					}

					if ((i < fnt->num_kern_pairs) && (k->c2 == (char)letter2))
					{
						*spacing += k->offset;
					}
				}
			}
		}

		return letter;
	}
}

int gr_get_font_height()
{
	if (FontManager::isReady())
	{
		return fl2i(FontManager::getCurrentFont()->getHeight());
	}
	else
	{
		return 16;
	}
}

int gr_get_dynamic_font_lines(int number_default_lines) {
	return fl2i((number_default_lines * 10) / (gr_get_font_height() + 1));
}

void gr_get_string_size(int *w1, int *h1, const char *text, int len)
{
	if (!FontManager::isReady())
	{
		if (w1)
			*w1 = 16;

		if (h1)
			*h1 = 16;

		return;
	}

	float w = 0.0f;
	float h = 0.0f;

	FontManager::getCurrentFont()->getStringSize(text, static_cast<size_t>(len), -1, &w, &h);

	if (w1)
	{
		*w1 = fl2i(ceil(w));
	}
	if (h1)
	{
		*h1 = fl2i(ceil(h));
	}
}

MONITOR(FontChars)

#ifdef _WIN32

void gr_string_win(int x, int y, char *s)
{
	using namespace font;

	int old_bitmap = gr_screen.current_bitmap;
	set_font(FONT1);
	gr_string(x, y, s);
	gr_screen.current_bitmap = old_bitmap;
}

#endif   // ifdef _WIN32

char grx_printf_text[2048];

void gr_printf(int x, int y, const char * format, ...)
{
	va_list args;

	if (!FontManager::isReady()) return;

	va_start(args, format);
	vsnprintf(grx_printf_text, sizeof(grx_printf_text) - 1, format, args);
	va_end(args);
	grx_printf_text[sizeof(grx_printf_text) - 1] = '\0';

	gr_string(x, y, grx_printf_text);
}

void gr_printf_menu(int x, int y, const char * format, ...)
{
	va_list args;

	if (!FontManager::isReady()) return;

	va_start(args, format);
	vsnprintf(grx_printf_text, sizeof(grx_printf_text) - 1, format, args);
	va_end(args);
	grx_printf_text[sizeof(grx_printf_text) - 1] = '\0';

	gr_string(x, y, grx_printf_text, GR_RESIZE_MENU);
}

void gr_printf_menu_zoomed(int x, int y, const char * format, ...)
{
	va_list args;

	if (!FontManager::isReady()) return;

	va_start(args, format);
	vsnprintf(grx_printf_text, sizeof(grx_printf_text) - 1, format, args);
	va_end(args);
	grx_printf_text[sizeof(grx_printf_text) - 1] = '\0';

	gr_string(x, y, grx_printf_text, GR_RESIZE_MENU_ZOOMED);
}

void gr_printf_no_resize(int x, int y, const char * format, ...)
{
	va_list args;

	if (!FontManager::isReady()) return;

	va_start(args, format);
	vsnprintf(grx_printf_text, sizeof(grx_printf_text) - 1, format, args);
	va_end(args);
	grx_printf_text[sizeof(grx_printf_text) - 1] = '\0';

	gr_string(x, y, grx_printf_text, GR_RESIZE_NONE);
}

#pragma once

#include "globalincs/pstypes.h"

#include "graphics/software/FSFont.h"
#include "graphics/software/FontManager.h"

namespace font
{	
	const int FONT1 = 0;	//<! The first loaded font. Used to be hardcoded to reference font01.vf in retail
	const int FONT2 = 1;	//<! The second loaded font. Used to be hardcoded to reference font02.vf in retail
	const int FONT3 = 2;	//<! The third loaded font. Used to be hardcoded to reference font03.vf in retail
	
	struct font;

	/**
	* @brief Parses the first font name
	*
	* Parses the first font name in the font table and stuffs it into @c firstFont
	*
	* @param firstFont The SCP_string which should contain the name
	*/
	void stuff_first(SCP_string &firstFont);

	/**
	* Crops a string if required to force it to not exceed max_width pixels when printed.
	* Does this by dropping characters at the end of the string and adding '...' to the end.
	*
	* @param str		string to crop.  Modifies this string directly
	* @param max_str	max characters allowed in str
	* @param max_width number of pixels to limit string to (less than or equal to).
	* @return			The width of the string
	*/
	int force_fit_string(char *str, int max_str, int max_width);

	/**
	* @brief Inites the font system
	*
	* Initializes the font system by setting up the FontManager, parse the font table(s) and
	* set the current font id to 0.
	*/
	void init();

	/**
	* @brief Closes the Font system
	*
	* Deallocates all allocated memory for the fonts and the respective font data.
	*/
	void close();

	/**
	* Retrieves the font which is located at index @c font_num and sets this font
	* as the current font
	* @param font_num The new font number, may not be an illegal font number
	*/
	inline void set_font(int fontnum)
	{
		FontManager::setCurrentFont(FontManager::getFont(fontnum));
	}

	/**
	* @brief Parses a font number
	*
	* Parses a font using either the font name or the font index and returns the font index
	* If the font could not be parsed because of a syntax error, -1 is returned instead
	*
	* @return The font index or -1 on error
	*/
	int parse_font();

	/**
	* @brief The currently active font index
	* @return The font index or -1 when the FontManager hasnt't been initialized yet
	*/
	inline int get_current_fontnum()
	{
		return FontManager::getCurrentFontIndex();
	}

	/**
	* @brief The current font object
	*
	* @return The current font object or NULL when not yet ready
	*/
	FSFont *get_current_font();

	/**
	* @brief Retrieves a font by index
	*
	* Gets a font by index into the internal font vector
	*
	* @param fontNum The index which should be returned
	* @return A font pointer or NULL of the index is not valid
	*/
	inline FSFont *get_font(int fontNum)
	{
		return FontManager::getFont(fontNum);
	}

	/**
	* @brief Retrieves a font by name
	*
	* @param name The name which should be searched
	* @return The font pointer or NULL if no font with that name could be found
	*/
	FSFont *get_font(const SCP_string& name);

	/**
	* @brief Retrieves a font by filename 
	*
	* @param filename The filename which should be searched
	* @return The font pointer or nullptr if no font with that filename could be found
	*/
	FSFont *get_font_by_filename(const SCP_string& filename);
}

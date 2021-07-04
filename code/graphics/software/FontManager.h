#pragma once

#include "globalincs/pstypes.h"

#include <memory>

namespace font {
	/**
	* @struct	TrueTypeFontData
	*
	* @brief	True type font data to save data for already read fonts.
	*
	* Used to store the data of a true type font which can be used by multiple FTFont objects which are all using the same data.
	*
	* @author	m!m
	* @date	24.11.2011
	*/
	struct TrueTypeFontData {
		size_t size;        //<! Size of allocated memory
		std::unique_ptr<ubyte[]> data;    //<! Allocated font data

		TrueTypeFontData() : size(0) { }

		TrueTypeFontData(TrueTypeFontData &&other) noexcept {
			std::swap(data, other.data);
			size = other.size;
		}

		TrueTypeFontData &operator=(TrueTypeFontData &&other) noexcept {
			std::swap(data, other.data);
			size = other.size;

			return *this;
		}

		TrueTypeFontData(const TrueTypeFontData &) = delete;
		TrueTypeFontData &operator=(const TrueTypeFontData &) = delete;
	};

	// Forward declarations
	class FSFont;

	class NVGFont;

	class VFNTFont;

	struct font;

	/**
	* @brief Manages the fonts used by FreeSpace
	*
	* This class is responsible for the creation, management and disposal of fonts used
	* by the FreeSpace engine. It contains static functions to load a specific font type
	* and functions to set or get the current font and can be used to retrieve a font by
	* name or index.
	*
	* @warning Don't @c delete a font which was retrieved from this class as they are managed internally
	*/
	class FontManager {
	public:
		/**
		* @brief Returns a pointer to the font with the specified name
		*
		* Searches the internal list of fonts for a font with the name @c name and returns it to the caller.
		* If the specified name could not be found then @c NULL is returned instead.
		*
		* @param name The name that should be searched for
		* @return The font pointer or @c NULL when font could not be found.
		*/
		static FSFont *getFont(const SCP_string &name);

		/**
		* @brief Returns a pointer to the font with the specified filename
		*
		* Searches the internal list of fonts for the _first_ font with the filename and returns it to the caller.
		* If the specified name could not be found then nullptr is returned instead.
		*
		* @param filename The filename that should be searched for
		* @return The font pointer or nullptr when font could not be found.
		*/
		static FSFont* getFontByFilename(const SCP_string &filename);

		/**
		* @brief Returns a pointer to the font at the specified index
		*
		* Returns the font pointer which is located as the specified index or @c NULL when the specified index is invald
		*
		* @param index The index that should be returns
		* @return Font pointer
		*/
		inline static FSFont *getFont(int index) {
			Assertion(index >= 0 && index < (int) fonts.size(), "Invalid font index %d given!", index);

			return fonts[index].get();
		}

		/**
		* @brief Returns the index of the currently used font
		*
		* Returns the index the currently active font pointer has.
		*
		* @return The current font index. 0 by default, -1 when FontManager isn't initialized yet
		*/
		static int getCurrentFontIndex();

		/**
		* @brief Returns a pointer to the current font
		*
		* Returns a pointer of the currently active font.
		*
		* @return Pointer of currently active font or @c NULL when no font is currently active
		*/
		static FSFont *getCurrentFont();


		/**
		* @brief Returns the index of the font with the specified @c name.
		*
		* @param name The name which should be searched
		* @return The index or -1 when font could not be found
		*/
		static int getFontIndex(const SCP_string &name);

		/**
		* @brief Returns the index of the specified font pointer
		*
		* Searches through the internal font vector and returns the index for which @verbatim (*fontIterator) == font @endverbatim
		* is true.
		*
		* @param font The font which should be searched
		* @return The index of the font or -1 when font could not be found
		*/
		static int getFontIndex(FSFont *font);

		/**
		* @brief Returns the number of fonts currently saved in the manager
		* @return The number of fonts
		*/
		static int numberOfFonts();

		/**
		* @brief Specifies if the font system is ready to be used for rendering text
		* @return @c true if ready (there is a current font), @c false if not
		*/
		static bool isReady();

		/**
		* @brief Checks if the specified number is a valid font ID
		* @return @c true if the number can be used with #getFont(int), @c false otherwise
		*/
		static bool isFontNumberValid(int fontNumber);

		/**
		* @brief Sets the currently active font
		* @param font The font which should be used, may not be @c NULL
		*
		* @warning The integrity of the specified pointer is not checked
		*/
		static void setCurrentFont(FSFont *font);

		/**
		* @brief Loads a TrueType font
		*
		* Loads a TrueType font with the specified @c fileName and initializes it with the specified
		* @c size and @c type. The size can be changed later where the type is not changeable.
		*
		* @param fileName The name of the font file which should be loaded
		* @param fontSize The initial size of the font
		* @param type The type of the font
		* @return A FTGLFont pointer or @c NULL when font could not be loaded
		*/
		static NVGFont *loadNVGFont(const SCP_string &fileName, float fontSize = 12.0f);

		/**
		* @brief Loads an old VFNT font
		*
		* Loads the VFNT font with the specified @c fileName and returns it in form of a VFNTFont pointer
		*
		* @param fileName The name of the font file
		* @return The font pointer or @c null on error
		*/
		static VFNTFont *loadVFNTFont(const SCP_string &fileName);

		/**
		* @brief Loads old volition font data
		*/
		static font *loadFontOld(const SCP_string& name);

		/**
		*	@brief Initializes the font system
		*/
		static void init();

		/**
		* @brief Disposes the fonts saved in the class
		*
		* Cleans up the memory used by the font system and sets the current font to @c NULL
		*/
		static void close();

	private:

		FontManager() { };

		FontManager(const FontManager &) { };

		~FontManager() { }

		static SCP_map<SCP_string, TrueTypeFontData> allocatedData;
		static SCP_map<SCP_string, std::unique_ptr<font>> vfntFontData;
		static SCP_vector<std::unique_ptr<FSFont>> fonts;

		static FSFont *currentFont;
	};
}

#pragma once

#include "globalincs/pstypes.h"

#include "graphics/2d.h"

#include <limits>

namespace font
{
	/**
	* @brief Enum to specify the type of a font
	*
	* Specifies the type of a font object and can be used to implement font specific behavior
	*/
	enum FontType
	{
		VFNT_FONT,		//!< A normal FreeSpace font
		NVG_FONT,		//!< A TrueType font as used by the NanoVG library
		UNKNOWN_FONT,	//!< An unknown font type. Probably means that an error happened
	};

	/**
	 * @brief	Abstract font class.
	 *
	 * An abstract class which is the superclass of every font class
	 *
	 * @author	m!m
	 * @date	23.11.2011
	 */
	class FSFont
	{
	private:
		SCP_string name;	//!< The name of this font
		SCP_string filename; //!< The file name used to retrieve this font

	protected:
		float offsetTop;		//!< The offset at the top of a line of text
		float offsetBottom;	//!< The offset at the bottom of a line of text

		float _height;
		float _ascender;
		float _descender;

		void checkFontMetrics();

	public:

		/**
		* @brief	Default constructor.
		*
		* @date	23.11.2011
		*/
		FSFont();

		/**
		* @brief	Destructor.
		*
		* @date	23.11.2011
		*/
		virtual ~FSFont();

		/**
		* @brief	Sets the name of this font.
		*
		* @date	23.11.2011
		*
		* @param	newName		The new name.
		*/
		void setName(const SCP_string& newName);

		/**
		* @brief	Sets the filename of this font.
		*
		* @date	9.9.2020
		*
		* @param	newName		The new filename.
		*/
		void setFilename(const SCP_string& newName);

		/**
		* @brief	Gets the name of this font.
		*
		* @date	23.11.2011
		*
		* @return	The name.
		*/
		const SCP_string& getName() const;

		/**
		* @brief	Gets the filename of this font.
		*
		* @date	9.9.2020
		*
		* @return	The name.
		*/
		const SCP_string& getFilename() const;

		/**
		* @brief	Gets the type of this font.
		*
		* The return value depends on the implementing subclass.
		*
		* @date	23.11.2011
		*
		* @return	The type.
		*
		* @see FontType::VFNT_FONT
		* @see FontType::FTGL_FONT
		*/
		virtual FontType getType() const = 0;

		/**
		* @brief	Gets the height of this font in pixels with regard to font top and bottom offsets.
		*
		* @date	23.11.2011
		*
		* @return	The height.
		*/
		float getHeight() const;

		/**
		* @brief	Gets the height of this font in pixels without the top and bottom offsets.
		*
		* @date	29.1.2012
		*
		* @return	The height.
		*/
		virtual float getTextHeight() const = 0;

		/**
		* @brief	Gets a string size.
		* Computes the size of the given string when it would be drawn by this font
		*
		* @date	23.11.2011
		*
		* @param [in]	text  	If non-null, the text.
		* @param textLen		Length of the text. Use -1 if the string should be checked until the next \0 character.
		* @param [out]	width 	If non-null, the width.
		* @param [out]	height	If non-null, the height.
		*/
		virtual void getStringSize(const char *text, size_t textLen = std::numeric_limits<size_t>::max(),
								   int resize_mode = -1, float *width = NULL, float *height = NULL) const = 0;

		/**
		* @brief	Gets the offset of this font from the top of the drawing line
		*
		* @date	27.11.2012
		*
		* @return	The top offset.
		*/
		float getTopOffset() const;

		/**
		* @brief	Gets the offset of this font from the bottom of the end of the text to where the next line will start.
		*
		* @date	27.1.2012
		*
		* @return	The bottom offset.
		*/
		float getBottomOffset() const;


		/**
		* @brief	Sets the top offset for this font
		*
		* @date	27.1.2012
		*
		* @param	newOffset The new top offset for this font
		*/
		void setTopOffset(float newOffset);


		/**
		* @brief	Sets the bottom offset for this font
		*
		* @date	27.1.2012
		*
		* @param	newOffset The new bottom offset for this font
		*/
		void setBottomOffset(float newOffset);

		/**
		 * @brief Recomputes the font metrics
		 */
		virtual void computeFontMetrics();

		/**
		 * @brief Gets the ascender value of this font
		 * @return The ascender value
		 */
		float getAscender();

		/**
		 * @breif Gets the descender value of this font
		 * @return The descender value
		 */
		float getDescender();
	};
}

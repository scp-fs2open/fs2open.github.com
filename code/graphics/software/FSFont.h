#pragma once

#include "globalincs/pstypes.h"

#include "graphics/2d.h"

#include <limits>

void removeFontMultiplierOption();

float get_font_scale_factor();

float calculate_auto_font_size(float current_size);

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
		SCP_string name = "<Invalid>";	//!< The name of this font
		SCP_string filename;			//!< The file name used to retrieve this font
		SCP_string familyName;          //!< The family name of the font. Will be "volition font" for bitmap fonts

	protected:
		bool autoScale = false;			//!< If the font is allowed to auto scale. Only used for VFNT fonts as NVG fonts do the auto scale calculation during parse time
		bool canScale = false;			//!< If the font is allowed to scale with the user font multiplier
		float offsetTop = 0.0f;			//!< The offset at the top of a line of text
		float offsetBottom = 0.0f;		//!< The offset at the bottom of a line of text

		float _height = 0.0f;
		float _ascender = 0.0f;
		float _descender = 0.0f;

		void checkFontMetrics();

	public:

		/**
		* @brief	Default constructor.
		*
		* @date	23.11.2011
		*/
		FSFont() = default;

		/**
		* @brief	Destructor.
		*
		* @date	23.11.2011
		*/
		virtual ~FSFont() = default;

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
		 * @brief	Sets the family name of this font.
		 *
		 * @date	17.6.2025
		 *
		 * @param	newName		The new famly name.
		 */
		void setFamilyName(const SCP_string& newName);

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
		 * @brief	Gets the family name of this font. Will be "Volition font" for bitmap fonts
		 *
		 * @date	17.6.2025
		 *
		 * @return	The family name.
		 */
		virtual const SCP_string& getFamilyName() const;

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
		* @param scaleMultiplier The scale to use to apply scaling in addition to user settings
		*/
		virtual void getStringSize(const char *text, size_t textLen = std::numeric_limits<size_t>::max(),
								   int resize_mode = -1, float *width = nullptr, float *height = nullptr, float scaleMultiplier = 1.0f) const = 0;

		/**
		* @brief    Gets the auto scaling behavior of this font
		*
		* @date     24.1.2025
		*
		* @return   The auto scaling behavior
		*/
		[[nodiscard]] bool getAutoScaleBehavior() const;

		/**
		 * @brief	Gets the scaling behavior of this font
		 *
		 * @date	28.8.2024
		 *
		 * @return	The scaling behavior
		 */
		[[nodiscard]] bool getScaleBehavior() const;

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
		* @brief    Sets the auto scaling behavior for VFNTs
		*
		* @date     24.1.2025
		*
		* @param    autoScale whether or not this font can auto scale with screen resolution
		*/
		void setAutoScaleBehavior(bool autoScale);

		/**
		 * @brief	Sets the scaling behavior
		 *
		 * @date	28.8.2024
		 *
		 * @param	scale whether or not this font can scale with the font multiplier
		 */
		void setScaleBehavior(bool scale);


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

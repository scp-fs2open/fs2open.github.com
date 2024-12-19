#pragma once

#include "globalincs/pstypes.h"
#include "graphics/software/FSFont.h"

void removeFontMultiplierOption();

namespace font
{
	struct font;

	/**
	* @brief	A VFNT font
	*
	* Class that contains a font of type FontType::VFNT_FONT
	* Name of the class is derived from the first four bytes of a font file which are "VFNT"
	*/
	class VFNTFont : public FSFont
	{
	private:
		font *fontPtr;  //!< The font pointer

	public:

		/**
		* @brief	Constructor that initializes the object with a primary font pointer
		*
		* @param [in]	fnt	A pointer to the font data. Has to be non-null
		*/
		explicit VFNTFont(font *fnt);

		/**
		* @brief	Destroys the allocated font pointer
		*/
		~VFNTFont() override;

		/**
		* @brief	Gets the font data struct.
		*
		* @return	the font data.
		*/
		font *getFontData();

		/**
		* @brief	Gets the type. This will always return FontType::VFNT_FONT
		*
		* @return	The type.
		*/
		FontType getType() const override;

		/**
		* @brief	Gets the height of this font
		*
		* @see FSFont::getHeight()
		*
		* @return	The height.
		*/
		float getTextHeight() const override;

		/**
		* @brief	Gets the size of the specified string in pixels.
		*
		* @param [in]	text  	the text which should be checked.
		* @param	textLen		  	Length of the text.
		* @param	resize_mode		The used resize mode
		* @param [out]	width 	If non-null, the width.
		* @param [out]	height	If non-null, the height.
		*/
		void
		getStringSize(const char* text, size_t textLen, int resize_mode, float* width, float* height) const override;
	};
}

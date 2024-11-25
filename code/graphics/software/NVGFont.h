#pragma once

#include "globalincs/pstypes.h"
#include "graphics/software/FSFont.h"

extern float Font_Scale_Factor;

namespace font
{
	struct font;
	class NVGFont : public FSFont
	{
	private:
		int m_handle = -1;
		float m_letterSpacing = 0.0f;
		float m_size = 12.0f;
		float m_tabWidth = 20.0f;

		font* m_specialCharacters = nullptr;

		float m_lineHeight = 0.0f;

	public:
		NVGFont() = default;
		~NVGFont() override = default;

		int getHandle() const { return m_handle; }
		float getSize() const { return m_size; }
		float getLetterSpacing() const { return m_letterSpacing; }
		float getTabWidth() const { return m_tabWidth; }
		font* getSpecialCharacterFont() const { return m_specialCharacters; }

		void setHandle(int handle);
		void setSize(float size);
		void setLetterSpacing(float spacing);
		void setTabWidth(float tabWidth);
		void setSpecialCharacterFont(font* fontData);

		FontType getType() const override { return NVG_FONT; };

		float getTextHeight() const override;

		void getStringSize(const char *text, size_t textLen, int resize_mode,
			float *width, float *height) const override;

		void computeFontMetrics() override;

		static size_t getTokenLength(const char *string, size_t maxLength);
	};
}

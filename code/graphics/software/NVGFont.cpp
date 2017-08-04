
#include "graphics/software/NVGFont.h"
#include "graphics/paths/PathRenderer.h"

#include "localization/localize.h"

#include <limits>

namespace
{
	const char* const TOKEN_SEPARATORS = "\n\t\r";
}

namespace font
{
	size_t NVGFont::getTokenLength(const char *string, size_t maxLength)
	{
		Assert(string != NULL);

		size_t length = 0;

		if (maxLength <= 0)
			return 0;

		if (*string >= Lcl_special_chars || *string < 0)
			return 1;

		const char *nullPtr = strchr(const_cast<char*>(string), '\0');
		const char *nextToken = strpbrk(const_cast<char*>(string), TOKEN_SEPARATORS);

		// WOHOO! Pointer arithmetic!!!
		if (nullPtr != NULL && (nextToken == NULL || nullPtr < nextToken))
		{
			length = nullPtr - string;
		}
		else if (nextToken != NULL)
		{
			if (nextToken == string)
			{
				length = 1;
			}
			else
			{
				length = nextToken - string;
			}
		}
		else
		{
			length = strlen(string);
		}

		if (length > maxLength)
		{
			length = maxLength;
		}

		for (size_t i = 0; i < length; i++)
		{
			if (string[i] >= Lcl_special_chars || string[i] < 0)
			{
				// Special character needs to be handled seperately
				return i;
			}
		}

		return length;
	}

	NVGFont::NVGFont() : m_handle(-1), m_letterSpacing(0.0f), m_size(12.0f), m_tabWidth(20.0f)
	{

	}

	NVGFont::~NVGFont()
	{

	}
	
	void NVGFont::setHandle(int handle)
	{
		Assertion(handle >= 0, "Invalid font handle passed!");
		m_handle = handle;
	}

	void NVGFont::setSize(float size)
	{
		Assertion(size > 0.f, "Invalid size %f passed!", size);
		m_size = size;
	}

	void NVGFont::setLetterSpacing(float spacing)
	{
		Assertion(spacing >= 0.0f, "Invalid letter spacing passed!");
		m_letterSpacing = spacing;
	}

	void NVGFont::setTabWidth(float tabWidth)
	{
		Assertion(tabWidth >= 0.0f, "Invalid tab width passed!");
		m_tabWidth = tabWidth;
	}

	void NVGFont::setSpecialCharacterFont(font* fontData)
	{
		Assertion(fontData != nullptr, "Invalid font data pointer passed!");
		m_specialCharacters = fontData;
	}
	
	float NVGFont::getTextHeight() const
	{
		return m_lineHeight;
	}

	extern int get_char_width_old(font* fnt, ubyte c1, ubyte c2, int *width, int* spacing);
	void NVGFont::getStringSize(const char *text, size_t textLen, int resize_mode, float *width, float *height) const
	{
		using namespace graphics::paths;

		auto path = PathRenderer::instance();

		path->saveState();
		path->resetState();

		path->fontFaceId(m_handle);
		path->fontSize(m_size);
		path->textLetterSpacing(m_letterSpacing);
		path->textAlign(static_cast<TextAlign>(ALIGN_TOP | ALIGN_LEFT));

		if (resize_mode != -1)
		{
			float scaleX = 1.0f;
			float scaleY = 1.0f;

			gr_resize_screen_posf(nullptr, nullptr, &scaleX, &scaleY, resize_mode);

			path->scale(scaleX, scaleY);
		}

		float w = 0.0f;
		float h = this->getHeight();

		size_t tokenLength;

		const char *s = text;
		bool specialChar = false;
		float lineWidth = 0.0f;

		while ((tokenLength = getTokenLength(s, textLen)) > 0)
		{
			if (tokenLength > textLen)
			{
				tokenLength = textLen;
			}

			if (tokenLength == 1)
			{
				// We may have encoutered a special character
				switch (*s)
				{
				case '\n':
					specialChar = true;

					h += this->getHeight();
					lineWidth = 0.0f;
					break;
				case '\t':
					specialChar = true;

					lineWidth += this->getTabWidth();
					break;
				default:
					if (*s >= Lcl_special_chars || *s < 0)
					{
						specialChar = true;

						int charWidth;
						int spacing;

						if (m_specialCharacters == nullptr) {
							Error(LOCATION,
								  "Font %s has no special characters font! This is usually caused by ignoring a font table parsing warning.",
								  getName().c_str());
						}

						get_char_width_old(m_specialCharacters, static_cast<ubyte>(*s), '\0', &charWidth, &spacing);

						lineWidth += i2fl(spacing);
					}
					break;
				}
			}

			if (!specialChar)
			{
				lineWidth += path->textBounds(0.f, 0.f, s, s + tokenLength, nullptr);
			}

			w = MAX(w, lineWidth);

			specialChar = false;

			s = s + tokenLength;

			textLen -= tokenLength;

			// see above, tokenLength is at most equal to textLen so this == 0 comparision should be safe
			if (textLen == 0)
			{
				break;
			}
		}

		if (height)
			*height = h;

		if (width)
			*width = w;

		path->restoreState();
	}
	void NVGFont::computeFontMetrics() {
		auto path = graphics::paths::PathRenderer::instance();

		path->saveState();
		path->resetState();

		path->fontFaceId(m_handle);
		path->fontSize(m_size);
		path->textLetterSpacing(m_letterSpacing);

		path->textMetrics(&_ascender, &_descender, &m_lineHeight);

		path->restoreState();

		_height = m_lineHeight + this->offsetTop + this->offsetBottom;

		checkFontMetrics();
	}
}
